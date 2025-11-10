#define motord 3
#define motore 5
void setup() {
  Serial.begin(9600);
  pinMode(motore,OUTPUT);
  pinMode(motord,OUTPUT);
}

void loop() {
if (Serial.available()>0) {  
String s = Serial.readStringUntil('\n');
Serial.println(s);
String distd=s.substring(10,15);
double aux1=distd.toDouble();
String diste=s.substring(16);
double aux2=diste.toDouble();
Serial.println(aux1);
Serial.println(aux2);
if(aux1<=2.00f){
  aux1 =(aux1/2.00f)*(-255.0f)+255.0f;
  analogWrite(motord,aux1);
}else{
  digitalWrite(motord,LOW);
}
if(aux2<=2.00f){
  aux2 =(aux2/2.00f)*(-255.0f)+255.0f;
  analogWrite(motore,aux2);
}else{
  digitalWrite(motore,LOW);
}
}

}