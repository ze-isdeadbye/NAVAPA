#define trigd 3 
#define echod 2
#define trige 5 
#define echoe 4
#define trigf 7
#define echof 6
const long veloSom=344.0; //para temperatura de 20 graus 
double dist=0.0;
long unsigned int ultMed;

double medeDist(int trig, int echo) { 
  double distance;
  long duration; 
   
  digitalWrite(trig, LOW); 
  delayMicroseconds(2); 
  digitalWrite(trig, HIGH); 
  delayMicroseconds(10); 
  digitalWrite(trig, LOW); 
  
  duration=pulseIn(echo,HIGH,30000);// timeout 30 ms (~5m)
  if (duration == 0) return 5.0; // sem eco, é mais de 5m então assume 5m

  distance = ((double)duration*veloSom)/(2000000.0f); 
  return distance;
}

void setup() {
  Serial.begin (9600);
  Serial1.begin(9600,SERIAL_8N1,10,9); 
  pinMode(trigd, OUTPUT); 
  pinMode(echod, INPUT);
  pinMode(trige, OUTPUT); 
  pinMode(echoe, INPUT);
  pinMode(trigf, OUTPUT); 
  pinMode(echof, INPUT);
  ultMed=millis();
}

void loop() {
  if(millis()-ultMed>=300){
  double distd=medeDist(trigd,echod);
  double diste=medeDist(trige,echoe);
  double distf=medeDist(trigf,echof);
  Serial1.print("distancia: ");Serial1.print(distd);Serial1.print(", ");Serial1.print(diste);Serial1.print(", ");Serial1.println(distf);
  ultMed=millis();
}
}
