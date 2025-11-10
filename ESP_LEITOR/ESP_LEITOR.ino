#define trigPin 3 
#define echoPin 2
const long veloSom=344.0; //para temperatura de 20 graus 
double dist=0.0;
long unsigned int ultMed;
double medeDist() { 
  double distance;
  long duration; 
  //Send pulse 
  digitalWrite(trigPin, LOW); 
  delayMicroseconds(2); 
  digitalWrite(trigPin, HIGH); 
  delayMicroseconds(10); 
  digitalWrite(trigPin, LOW); 
  //Wait for echo and measure time until it happens
  duration=pulseIn(echoPin,HIGH);
  //Compute distance 
  distance = ((double)duration*veloSom)/(2000000.0f); 
  return distance;
}
void setup() {
  Serial.begin (9600); 
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT);
  ultMed=millis();
}

void loop() {
  if(millis()-ultMed>=300){
  double dist=medeDist();
  Serial.print("distancia: ");Serial.println(dist);
  ultMed=millis();
}
}
