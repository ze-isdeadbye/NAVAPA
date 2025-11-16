#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Credenciais do AP criado pelo recetor
const char* ssid     = "NAVAPA";
const char* password = "AfonsoJose";

// Endereço do endpoint
const char* serverURL = "http://192.168.4.1/distances";

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long int ultimamed=0;
void setup() {
  Serial.begin(9600);
  delay(100);

  // Liga-te ao Wi-Fi
  Serial.printf("A ligar ao SSID “%s” …\n", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi ligado!");
  Serial.print("IP local: ");
  Serial.println(WiFi.localIP());

  // Inicializa display OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Falha no SSD1306");
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("A iniciar...");
  display.display();
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

      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Direita : "); display.print(distD, 2); display.println(" m");
      display.print("Esquerda: "); display.print(distE, 2); display.println(" m");
      display.print("Frente  : "); display.print(distF, 2); display.println(" m");
      display.print("Modo    : "); display.println(modoStr);
      display.display();
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