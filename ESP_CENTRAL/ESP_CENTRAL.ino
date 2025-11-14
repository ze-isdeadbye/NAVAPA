#define motord 3
#define motore 5

unsigned long proximoPulsoD = 0;
unsigned long proximoPulsoE = 0;
bool estadoMotorD = false;
bool estadoMotorE = false;

void setup() {
  Serial.begin(9600);
  pinMode(motore,OUTPUT);
  pinMode(motord,OUTPUT);
}

void loop() {
  double aux1=5.00,aux2=5.00,aux3=5.00;
if (Serial.available()>0) {  
String s = Serial.readStringUntil('\n');
Serial.println(s);
String distd=s.substring(10,15);
aux1=distd.toDouble();
String diste=s.substring(16,21);
aux2=diste.toDouble();
String distf = s.substring(22);
aux3= distf.toDouble();
Serial.println(aux1);
Serial.println(aux2);
Serial.println(aux3);
if(aux1<=1.00f){
  proximoPulsoD=(aux1)*450+50;
  if(aux3<=1.00f){
    proximoPulsoD=max(proximoPulsoD*(aux3),50);
  }
} 
if(aux2<=1.00f){
  proximoPulsoE=(aux2)*450+50;
  if(aux3<=1.00f){
    proximoPulsoE=max(proximoPulsoE*(aux3),50);
  }
}
if (aux1>1.00f&&aux2>1.00f&&aux3<=1.00f){
  proximoPulsoE=(aux3)*450+50;
  proximoPulsoD=(aux3)*450+50;
}

}
static unsigned long tD = 0;
  if (aux1 <= 1.00f||aux3<=1.00f) {
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
  if (aux2 <= 1.00f||aux3<=1.00f) {
    if (millis() - tE >= proximoPulsoE) {
      estadoMotorE = !estadoMotorE;
      digitalWrite(motore, estadoMotorE);
      tE = millis();
    }
  } else {
    digitalWrite(motore, LOW); // desligar se > 1m
    estadoMotorE = false;
  }
}