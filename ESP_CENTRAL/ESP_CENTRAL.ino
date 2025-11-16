#include <WiFi.h>
#include <WebServer.h>
#define motord 3
#define motore 5
#define botao 4
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
double aux1=5.00,aux2=5.00,aux3=5.00;

const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>Visão Tipo Sonar</title>
<style>
  body { background:#111; color:#0f0; text-align:center; }
  canvas { background:#222; margin-top:20px; }
</style>
</head>
<body>

<h1>Visão Tipo Sonar</h1>
<canvas id="radar"></canvas>

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
    fetch('/distances')
      .then(resp => resp.json())
      .then(data => {
        desenha(data.distE, data.distF, data.distD);
      })
      .catch(err => console.error("Erro no fetch:", err));
  }

  atualiza();
  setInterval(atualiza, 500);
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
  json += "\"distD\":" + String(aux1, 2) + ",";
  json += "\"distE\":" + String(aux2, 2) + ",";
  json += "\"distF\":" + String(aux3, 2) + ",";
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
  pinMode(botao,INPUT_PULLUP);
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
    String distd=s.substring(10,15);
    aux1=distd.toDouble();
    String diste=s.substring(16,21);
    aux2=diste.toDouble();
    String distf = s.substring(22);
    aux3= distf.toDouble();
    if(aux1<=distanciaSeguranca){
      if(mode==interior){proximoPulsoD=(aux1)*450+50;}
      else proximoPulsoD=(aux1)/2.00f*450+50;
      if(aux3<=distanciaSeguranca){
        if(mode==interior)proximoPulsoD=(proximoPulsoD*(aux3)>50)?proximoPulsoD*(aux3):50;
        else proximoPulsoD=(proximoPulsoD*(aux3)/2.00f>50)?proximoPulsoD*(aux3)/2.00f:50;
      }
    } 
    if(aux2<=distanciaSeguranca){
      if(mode==interior)proximoPulsoE=(aux2)*450+50;
      else proximoPulsoE=(aux2)/2.00f*450+50;
      if(aux3<=distanciaSeguranca){
        if(mode==interior)proximoPulsoE=(proximoPulsoE*(aux3)>50)?proximoPulsoE*(aux3):50;
        else proximoPulsoE=(proximoPulsoE*(aux3)/2.00f>50)?proximoPulsoE*(aux3)/2.00f:50;
      }
    }
    if (aux1>distanciaSeguranca&&aux3<=distanciaSeguranca){
      if(mode==interior)proximoPulsoD=(aux3)*450+50;
      else proximoPulsoD=(aux3)/2.00f*450+50;
    }
    if (aux2>distanciaSeguranca&&aux3<=distanciaSeguranca){
      if(mode==interior)proximoPulsoE=(aux3)*450+50;
      else proximoPulsoE=(aux3)/2.00f*450+50;
    }
  }
  static unsigned long tD = 0;
    if (aux1 <= distanciaSeguranca||aux3<=distanciaSeguranca) {
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
    if (aux2 <= distanciaSeguranca||aux3<=distanciaSeguranca) {
      if (millis() - tE >= proximoPulsoE) {
        estadoMotorE = !estadoMotorE;
        digitalWrite(motore, estadoMotorE);
        tE = millis();
      }
    } else {
      digitalWrite(motore, LOW); 
      estadoMotorE = false;
    }
    if(digitalRead(botao)==HIGH){
      mode=interior;
      distanciaSeguranca=1.00f;
    }else{
      mode=exterior;
      distanciaSeguranca=2.00f;
    }
}