#define trigd 3 
#define echod 2
#define trige 5 
#define echoe 4
const long veloSom=344.0; //para temperatura de 20 graus 
double dist=0.0;
long unsigned int ultMed;
double medeDistd() { 
  double distance;
  long duration; 
  //Send pulse 
  digitalWrite(trigd, LOW); 
  delayMicroseconds(2); 
  digitalWrite(trigd, HIGH); 
  delayMicroseconds(10); 
  digitalWrite(trigd, LOW); 
  //Wait for echo and measure time until it happens
  duration=pulseIn(echod,HIGH);
  //Compute distance 
  distance = ((double)duration*veloSom)/(2000000.0f); 
  return distance;
}
double medeDiste() { 
  double distance;
  long duration; 
  //Send pulse 
  digitalWrite(trige, LOW); 
  delayMicroseconds(2); 
  digitalWrite(trige, HIGH); 
  delayMicroseconds(10); 
  digitalWrite(trige, LOW); 
  //Wait for echo and measure time until it happens
  duration=pulseIn(echoe,HIGH);
  //Compute distance 
  distance = ((double)duration*veloSom)/(2000000.0f); 
  return distance;
}
void setup() {
  Serial.begin (9600); 
  pinMode(trigd, OUTPUT); 
  pinMode(echod, INPUT);
  pinMode(trige, OUTPUT); 
  pinMode(echoe, INPUT);
  ultMed=millis();
}

void loop() {
  if(millis()-ultMed>=300){
  double distd=medeDistd();
  double diste=medeDiste();
  Serial.print("distancia: ");Serial.print(distd);Serial.print(", ");Serial.println(diste);
  ultMed=millis();
}
}
