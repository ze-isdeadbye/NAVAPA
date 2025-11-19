#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <LiquidCrystal.h>

#define cs   4 
#define dc   3 
#define rst  2
#define mosi 7
#define sck 6

Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, mosi , sck, rst);

#define RS 5
#define E 8
#define D4 23
#define D5 22
#define D6 21
#define D7 20

LiquidCrystal lcd(RS, E, D4, D5, D6, D7);

const char* ssid     = "NAVAPA";
const char* password = "AfonsoJose";

// Endereço do endpoint
const char* serverURL = "http://192.168.4.1/distances";

unsigned long int ultimamed=0;

double distD = 5.0, distE = 5.0, distF = 5.0;
int modo = 0;
double temp = 0.0;
int passos = 0;

void setup() {
  Serial.begin(9600);
  delay(100);

  lcd.begin(16, 2);

  tft.initR(INITR_GREENTAB);
  tft.setSPISpeed(27000000);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(0xFFFFFF);
  tft.setTextSize(1);
  tft.setCursor(10, 10);

  Serial.printf("A ligar ao SSID “%s” …\n", ssid);
  tft.println("A ligar ao AP!");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    tft.print(".");
  }
  Serial.println("\nWi-Fi ligado!");
  tft.println("\nLigado ao AP!"); 
  Serial.print("IP local: ");
  Serial.println(WiFi.localIP());

}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if(millis()-ultimamed>=200){
    HTTPClient http;
    http.begin(serverURL);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      String leituras = http.getString();
      String saux;

      int Pontos = leituras.indexOf("distD\":");
      int Virgula = leituras.indexOf(',', Pontos);
      saux = leituras.substring(Pontos + 7, Virgula);
      distD = saux.toDouble();

      Pontos = leituras.indexOf("distE\":");
      Virgula = leituras.indexOf(',', Pontos);
      saux = leituras.substring(Pontos + 7, Virgula);
      distE = saux.toDouble();

      Pontos= leituras.indexOf("distF\":");
      Virgula = leituras.indexOf(',', Pontos);
      saux = leituras.substring(Pontos + 7, Virgula);
      distF = saux.toDouble();

      Pontos = leituras.indexOf("passos\":");
      Virgula = leituras.indexOf("," , Pontos);
      saux = leituras.substring(Pontos + 8, Virgula);
      passos = saux.toInt();

      Pontos = leituras.indexOf("temperatura\":");
      Virgula = leituras.indexOf("," , Pontos);
      saux = leituras.substring(Pontos + 13, Virgula);
      temp = saux.toDouble();
      
      Pontos = leituras.indexOf("modo\":");
      Virgula = leituras.indexOf(',', Pontos);
      saux = leituras.substring(Pontos + 6, Virgula);
      saux.trim();
      modo = saux.toInt();  // 0 = interior, 1 = exterior

      Pontos = leituras.indexOf("queda\":");
      Virgula = leituras.indexOf("}", Pontos);
      String q = leituras.substring(Pontos + 7, Virgula);

      if (q.equalsIgnoreCase("true")){
        Serial.printf("QUEDA");
      }else{
        const char* modoStr = (modo == 0 ? "Interior" : "Exterior");
        Serial.printf("Direita: %.2fm   Esquerda: %.2fm   Frente: %.2fm   Modo: %s\n",distD, distE, distF,modoStr);
        lcd.setCursor(0, 0);
        lcd.print("Dists: (E;F;D)m");
        lcd.setCursor(0, 1);
        String saux2 = String("(")+String(distE)+String(",")+String(distF)+String(",")+String(distD)+String(")");
        char s[18];
        saux2.toCharArray(s,17);
        lcd.print(s); 
      }
    }
    else {
      Serial.printf("Erro HTTP: %d\n", httpCode);
    }
    http.end();
    ultimamed=millis();
  }
  }else {
    Serial.println("Wi-Fi perdido, a tentar reconectar …");
    /*WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }*/
    WiFi.reconnect(); 
  }
}