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

const char* ssid     = "NAVAPA";
const char* password = "AfonsoJose";

WebServer server(80);

unsigned long proximoPulsoD = 0;
unsigned long proximoPulsoE = 0;
bool estadoMotorD = false;
bool estadoMotorE = false;
double aux1=5.00,aux2=5.00,aux3=5.00;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600,SERIAL_8N1,21,20);
  pinMode(motore,OUTPUT);
  pinMode(motord,OUTPUT);
  pinMode(botao,INPUT);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  Serial.println("AP ativo. SSID: " + String(ssid));
  Serial.print("IP do AP: ");
  Serial.println(WiFi.softAPIP());
  server.on("/distances", HTTP_GET, []() {
    // constroi JSON simples
    String json = "{";
    json += "\"distD\":" + String(aux1, 2) + ",";
    json += "\"distE\":" + String(aux2, 2) + ",";
    json += "\"distF\":" + String(aux3, 2);
    json += "}";
    server.send(200, "application/json", json);
  });
  server.begin();
  mode=interior;
}

void loop() {
  server.handleClient();
if (Serial1.available()>0) {  
String s = Serial1.readStringUntil('\n');
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

  // vibração do motor esquerdo
  static unsigned long tE = 0;
  if (aux2 <= distanciaSeguranca||aux3<=distanciaSeguranca) {
    if (millis() - tE >= proximoPulsoE) {
      estadoMotorE = !estadoMotorE;
      digitalWrite(motore, estadoMotorE);
      tE = millis();
    }
  } else {
    digitalWrite(motore, LOW); // desligar se > 1m
    estadoMotorE = false;
  }
  if(digitalRead(botao)==HIGH){
    mode=(mode==interior?exterior:interior);
    distanciaSeguranca=(mode==interior?1.00f:2.00f);
  }
}