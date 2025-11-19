#include <WiFi.h>
#include <WebServer.h>
#define motord 3
#define motore 5
#define botaoInterior 18
#define botaoExterior 19

enum modooperacao{
  interior,
  exterior
}mode;

double distanciaSeguranca=1.00f;
bool quedaDetectada = false; 
int numPassos = 0;
double temperatura = 0.0;

const char* ssid     = "NAVAPA";
const char* password = "AfonsoJose";

WebServer server(80);

unsigned long proximoPulsoD = 0;
unsigned long proximoPulsoE = 0;
bool estadoMotorD = false;
bool estadoMotorE = false;
double DIR=5.00,ESQ=5.00,FREN=5.00; 


const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>Visão Tipo Sonar</title>
<style>
  body { background:#111; color:#0f0; text-align:center; }
  canvas { background:#222; margin-top:20px; }
  @keyframes piscar {
  0%   { background-color: #800; }
  50%  { background-color: #f00; }
  100% { background-color: #800; }
}
</style>
</head>
<body>

<h1>Visão Tipo Sonar</h1>

<div id="alertaQueda" style="
    display:none;
    font-size: 2em;
    font-weight: bold;
    color: white;
    padding: 20px;
    margin-top: 20px;
    border-radius: 10px;
    animation: piscar 0.8s infinite;
">
⚠ O USUÁRIO CAIU!
</div>

<canvas id="radar"></canvas>

<!-- PAINEL DE INFORMAÇÃO -->
<div id="infoPanel" style="
    margin-top: 20px;
    color: #0f0;
    font-size: 1.3em;
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 10px;
">

  <div id="distancias" style="
      width: 90%;
      display: flex;
      justify-content: space-between;
      font-size: 1.4em;
      padding: 10px;
      border: 1px solid #0f0;
      border-radius: 10px;
  ">
      <div>E: <span id="dE">0</span> m</div>
      <div>F: <span id="dF">0</span> m</div>
      <div>D: <span id="dD">0</span> m</div>
  </div>

  <div id="extraInfo" style="
      width: 90%;
      display: flex;
      justify-content: space-between;
      font-size: 1.2em;
      padding: 10px;
      border: 1px solid #0f0;
      border-radius: 10px;
  ">
      <div>Passos: <span id="passos">0</span></div>
      <div>Temp: <span id="temp">0</span> °C</div>
      <div>Modo: <span id="modo">?</span></div>
  </div>

</div>


<script>
  const canvas = document.getElementById("radar");
  const ctx = canvas.getContext("2d");

  function ajustaCanvas() {
    const w = Math.min(window.innerWidth * 0.9, 800);
    const h = w * 0.75;
    canvas.width  = w;
    canvas.height = h;
  }
  ajustaCanvas();
  window.addEventListener("resize", ajustaCanvas);

  function desenha(distE, distF, distD) {
    const cx = canvas.width / 2;
    const cy = canvas.height - 10;

    ctx.clearRect(0, 0, canvas.width, canvas.height);

    const CENTRO = Math.PI * 1.5;
    const centrais = [
      CENTRO - 0.5,  // esquerda
      CENTRO,        // frente
      CENTRO + 0.5   // direita
    ];

    const abertura = 0.5; // ~30º
    const maxDist = 2;
    const dists = [distE, distF, distD];

    for (let i = 0; i < 3; i++) {
      const cen = centrais[i];
      const a1 = cen - abertura / 2;
      const a2 = cen + abertura / 2;

      const raio = (Math.min(dists[i], maxDist) / maxDist) * (canvas.height - 20);

      ctx.beginPath();
      ctx.moveTo(cx, cy);
      ctx.arc(cx, cy, canvas.height - 20, a1, a2);
      ctx.closePath();
      ctx.fillStyle = "#800";
      ctx.fill();

      ctx.beginPath();
      ctx.moveTo(cx, cy);
      ctx.arc(cx, cy, raio, a1, a2);
      ctx.closePath();
      ctx.fillStyle = "#0f0";
      ctx.fill();
    }
  }

  function atualiza() {
  fetch('/distances?' + Date.now())  // evita cache
    .then(resp => resp.json())
    .then(data => {

      // Atualiza o radar
      desenha(data.distE, data.distF, data.distD);

      // Atualiza alerta de queda
      const alerta = document.getElementById("alertaQueda");
      alerta.style.display = data.queda ? "block" : "none";

      // Atualiza valores no painel
      document.getElementById("dD").innerText = data.distD.toFixed(2);
      document.getElementById("dE").innerText = data.distE.toFixed(2);
      document.getElementById("dF").innerText = data.distF.toFixed(2);

      document.getElementById("passos").innerText = data.passos;
      document.getElementById("temp").innerText = data.temperatura.toFixed(2);

      document.getElementById("modo").innerText =
          data.modo === 0 ? "Interior" : "Exterior";
    })
    .catch(err => console.error("Erro no fetch:", err));
}


  atualiza();
  setInterval(atualiza, 200);
</script>

</body>
</html>
)rawliteral"; 

void handleRoot() { 
  String page = htmlPage; 
  server.send(200, "text/html", page); 
} 
void handleDists(){
  String json = "{";
  json += "\"distD\":" + String(DIR, 2) + ",";
  json += "\"distE\":" + String(ESQ, 2) + ",";
  json += "\"distF\":" + String(FREN, 2) + ",";
  json += "\"passos\":" + String(numPassos) + ",";
  json += "\"temperatura\":" + String(temperatura, 2) + ",";
  json += "\"modo\":" + String(mode) + ",";
  json += "\"queda\":" + String(quedaDetectada ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}
void setup() {
  Serial.begin(9600);
  Serial1.begin(9600,SERIAL_8N1,21,20);
  
  pinMode(motore,OUTPUT);
  pinMode(motord,OUTPUT);
  pinMode(botaoExterior,INPUT_PULLUP);
  pinMode(botaoInterior,INPUT_PULLUP);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  Serial.println("AP ativo. SSID: " + String(ssid));
  Serial.print("IP do AP: ");
  Serial.println(WiFi.softAPIP());
  server.on("/", handleRoot);
  server.on("/distances", handleDists);
  server.begin();
  mode=interior;
}

void loop() {
  server.handleClient();
  if (Serial1.available()>0) {  
    String s = Serial1.readStringUntil('\n');

    if (s.equalsIgnoreCase("QUEDA")) {
      quedaDetectada = true;

      digitalWrite(motord, LOW);
      digitalWrite(motore, LOW);
      estadoMotorD = false;
      estadoMotorE = false;

      Serial.println("** QUEDA DETETADA! **");

      return;
    }
    Serial.println(s);
    int doispts = s.indexOf(":",0);
    int priVirgula=s.indexOf(",",doispts);
    String distd=s.substring(doispts+1,priVirgula);
    DIR=distd.toDouble();

    int secVirgula=s.indexOf(",",priVirgula+1);
    String diste=s.substring(priVirgula+1,secVirgula);
    ESQ=diste.toDouble();

    int terVirgula = s.indexOf(",",secVirgula+1);
    String distf = s.substring(secVirgula+1,terVirgula);
    FREN= distf.toDouble();

    int quarVirgula = s.indexOf(",",terVirgula+1);
    String passos= s.substring(terVirgula+1,quarVirgula);
    numPassos = passos.toInt();

    int quinVirgula = s.indexOf(",", quarVirgula+1);
    String t = s.substring(quarVirgula+1,quinVirgula);
    temperatura = t.toDouble();

    if(DIR<=distanciaSeguranca){
      if(mode==interior){proximoPulsoD=(DIR)*450+50;}
      else proximoPulsoD=(DIR)/2.00f*450+50;
      if(FREN<=distanciaSeguranca){
        if(mode==interior)proximoPulsoD=(proximoPulsoD*(FREN)>50)?proximoPulsoD*(FREN):50;
        else proximoPulsoD=(proximoPulsoD*(FREN)/2.00f>50)?proximoPulsoD*(FREN)/2.00f:50;
      }
    } 
    if(ESQ<=distanciaSeguranca){
      if(mode==interior)proximoPulsoE=(ESQ)*450+50;
      else proximoPulsoE=(ESQ)/2.00f*450+50;
      if(FREN<=distanciaSeguranca){
        if(mode==interior)proximoPulsoE=(proximoPulsoE*(FREN)>50)?proximoPulsoE*(FREN):50;
        else proximoPulsoE=(proximoPulsoE*(FREN)/2.00f>50)?proximoPulsoE*(FREN)/2.00f:50;
      }
    }
    if (DIR>distanciaSeguranca&&FREN<=distanciaSeguranca){
      if(mode==interior)proximoPulsoD=(FREN)*450+50;
      else proximoPulsoD=(FREN)/2.00f*450+50;
    }
    if (ESQ>distanciaSeguranca&&FREN<=distanciaSeguranca){
      if(mode==interior)proximoPulsoE=(FREN)*450+50;
      else proximoPulsoE=(FREN)/2.00f*450+50;
    }
  }
  static unsigned long tD = 0;
    if (DIR <= distanciaSeguranca||FREN<=distanciaSeguranca) {
      if (millis() - tD >= proximoPulsoD) {
        estadoMotorD = !estadoMotorD;
        digitalWrite(motord, estadoMotorD);
        tD = millis();
      }
    } else {
      digitalWrite(motord, LOW); // desligar se > 1m
      estadoMotorD = false;
    }

  static unsigned long tE = 0;
    if (ESQ <= distanciaSeguranca||FREN<=distanciaSeguranca) {
      if (millis() - tE >= proximoPulsoE) {
        estadoMotorE = !estadoMotorE;
        digitalWrite(motore, estadoMotorE);
        tE = millis();
      }
    } else {
      digitalWrite(motore, LOW); 
      estadoMotorE = false;
    }
    if(digitalRead(botaoInterior)==LOW){
      mode=interior;
      distanciaSeguranca=1.00f;
    }
    if(digitalRead(botaoExterior)==LOW){
      mode=exterior;
      distanciaSeguranca=2.00f;
    }
}