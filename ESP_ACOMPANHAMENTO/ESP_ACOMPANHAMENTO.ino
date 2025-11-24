#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <LiquidCrystal.h>

#define botao 14

enum apresentacao{
  distanciaE,
  distanciaF,
  distanciaD,
  modoINTEXT,
  temperatura,
  numeroPassos
}aplcd;

byte aviso[] = { 
  B00100, 
  B01010, 
  B10101, 
  B10101, 
  B10101, 
  B10001, 
  B10101, 
  B11111 
}; 

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
int ultPE[2]={160,0};
int ultPF[2]={160,0};
int ultPD[2]={160,0};
int pontos=0;

void setup() {
  Serial.begin(9600);
  pinMode(botao,INPUT_PULLUP);
  delay(100);

  lcd.begin(16, 2);
  lcd.createChar(0, aviso);

  tft.initR(INITR_GREENTAB);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(0xFFFFFF);
  tft.setTextSize(1);
  tft.setCursor(10, 10);

  Serial.printf("A ligar ao SSID “%s” …\n", ssid);
  tft.setRotation(3);
  tft.println("A ligar ao AP!");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    tft.print(".");
  }
  Serial.println("\nWi-Fi ligado!");
  tft.println("\nLigado ao AP!"); 
  tft.setRotation(1);
  delay(1000);
  tft.fillScreen(ST77XX_BLACK);
  Serial.print("IP local: ");
  Serial.println(WiFi.localIP());

  aplcd=distanciaE;
}

void loop() {
  if(digitalRead(botao)==HIGH){
    switch (aplcd){
      case distanciaE:aplcd=distanciaF;break;
      case distanciaF:aplcd=distanciaD;break;
      case distanciaD:aplcd=modoINTEXT;break;
      case modoINTEXT:aplcd=temperatura;break;
      case temperatura:aplcd=numeroPassos;break;
      case numeroPassos:aplcd=distanciaE;break;
    }
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Info mostrada:");
    lcd.setCursor(0,1);
    switch (aplcd){
      case distanciaE:
      lcd.print("Distancia a ESQ");
      break;
      case distanciaF:
      lcd.print("Distancia a FRE");
      break;
      case distanciaD:
      lcd.print("Distancia a DIR");
      break;
      case modoINTEXT:
      lcd.print("Modo de atuacao");
      break;
      case temperatura:
      lcd.print("Temperatura");
      break;
      case numeroPassos:
      lcd.print("No. passos dados");
      break;
    }
    delay(1000);
    lcd.clear();
  }
  if (WiFi.status() == WL_CONNECTED) {
    if(millis()-ultimamed>=200){
    lcd.setCursor(0, 0);

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
      q.trim();
      if (q.equalsIgnoreCase("true")){
        Serial.printf("QUEDA");
        tft.fillScreen(ST77XX_BLACK);
        tft.setTextSize(1);
        tft.setTextColor(ST77XX_WHITE);
        tft.setRotation(3);
        tft.setCursor(10, 5);
        tft.println("Foi detetada uma queda"); tft.println(" do usuario do NAVAPA, que");tft.print(" podera precisar de ajuda!");
        tft.setRotation(1);
        tft.drawTriangle(50,40,110,40,80,88,0x07FF);
        tft.fillRect(78, 60, 4, 20, 0x07FF);
        tft.fillRect(78, 50, 4, 6, 0x07FF);
        lcd.clear();
        lcd.print("O USUARIO CAIU!");
        lcd.setCursor(0,1);
        for(int i=0; i<16;i++)lcd.write(byte(0));
        while(1){}
      }else{
        int alturaE = (distE/5.00f)*128;
        int alturaF = (distF/5.00f)*128;
        int alturaD = (distD/5.00f)*128;

        tft.drawLine(ultPE[0], ultPE[1], ultPE[0]+2, alturaE,ST77XX_BLUE ); //está a desnhar vermelho
        ultPE[0]-=2;
        ultPE[1]=alturaE;

        tft.drawLine(ultPF[0], ultPF[1], ultPF[0]+2, alturaF,ST77XX_GREEN );// está a desenhar verde
        ultPF[0]-=2;
        ultPF[1]=alturaF;

        tft.drawLine(ultPD[0], ultPD[1], ultPD[0]+2, alturaD,ST77XX_RED );//está a desenhar azul 
        ultPD[0]-=2;
        ultPD[1]=alturaD;

        pontos++;

        if(pontos==80){
          tft.fillScreen(ST77XX_BLACK);
          ultPE[0]=160;
          ultPF[0]=160;
          ultPD[0]=160;
          pontos=0;
        }
        switch (aplcd){
          case distanciaE:
          lcd.print("Distancia a ESQ:");
          lcd.setCursor(0, 1);
          lcd.print(distE,2);
          lcd.print(" m (Linha R)");
          break;
          case distanciaF:
          lcd.print("Distancia a FRE:");
          lcd.setCursor(0, 1);
          lcd.print(distF,2);
          lcd.print(" m (Linha G)");
          break;
          case distanciaD:
          lcd.print("Distancia a DIR:");
          lcd.setCursor(0, 1);
          lcd.print(distD,2);
          lcd.print(" m (Linha B)");
          break;
          case modoINTEXT:
          lcd.print("Modo de atuacao:");
          lcd.setCursor(0, 1);
          lcd.print(modo==0?"Esta no Interior":"Esta no Exterior");
          break;
          case temperatura:
          lcd.print("Temperatura: ");
          lcd.setCursor(0, 1);
          lcd.print(temp,2);
          lcd.print(temp<=15.00f?" C, Frio":(temp>=25.00f?" C, Calor":" C"));
          if (temp<=15.00f||temp>=25.00f)lcd.write(byte(0));
          break;
          case numeroPassos:
          lcd.print("No. passos dados:");
          lcd.setCursor(0, 1);
          lcd.print(passos);
          break;
        }
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