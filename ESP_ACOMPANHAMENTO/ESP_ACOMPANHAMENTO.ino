/*#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>*/
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <LiquidCrystal.h>

#define cs   4 
#define dc   3 
#define rst  2
#define mosi 7
#define sck 6

Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc,mosi , sck, rst);

LiquidCrystal lcd(5, 8, 23, 22, 21, 20);

const char* ssid     = "NAVAPA";
const char* password = "AfonsoJose";

// Endereço do endpoint
const char* serverURL = "http://192.168.4.1/distances";

unsigned long int ultimamed=0;
void setup() {
  Serial.begin(9600);
  delay(100);

  Serial.printf("A ligar ao SSID “%s” …\n", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi ligado!");
  Serial.print("IP local: ");
  Serial.println(WiFi.localIP());

  lcd.begin(16, 2);
  lcd.print("LCD + TFT OK!");

  tft.initR(INITR_GREENTAB);
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("ESP32-C6");
  tft.println("ST7735 OK!");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if(millis()-ultimamed>=300){
    HTTPClient http;
    http.begin(serverURL);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();

      float distD = 5.0, distE = 5.0, distF = 5.0;
      int modo = 0;
      int p, p2;
      String s;

      p = payload.indexOf("distD\":");
      if (p >= 0) {
        p2 = payload.indexOf(',', p);
        s = payload.substring(p + 7, p2);
        distD = s.toFloat();
      }

      p = payload.indexOf("distE\":");
      if (p >= 0) {
        p2 = payload.indexOf(',', p);
        s = payload.substring(p + 7, p2);
        distE = s.toFloat();
      }

      p = payload.indexOf("distF\":");
      if (p >= 0) {
        p2 = payload.indexOf('}', p);
        s = payload.substring(p + 7, p2);
        distF = s.toFloat();
      }
      p = payload.indexOf("modo\":");
      if (p >= 0) {
        p2 = payload.indexOf('}', p);
        s = payload.substring(p + 6, p2);
        s.trim();
        modo = s.toInt();  // 0 = interior, 1 = exterior
      }
      const char* modoStr = (modo == 0 ? "Interior" : "Exterior");
      // Imprime no Monitor Serial
      Serial.printf("Direita: %.2fm   Esquerda: %.2fm   Frente: %.2fm   Modo: %s\n",distD, distE, distF,modoStr);

    }
    else {
      Serial.printf("Erro HTTP: %d\n", httpCode);
    }
    http.end();
    ultimamed=millis();
  }
  }else {
    Serial.println("Wi-Fi perdido, a tentar reconectar …");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
  }
}