#include <Wire.h>
#include <BMI160Gen.h>
#define trigd 3 
#define echod 2
#define trige 5 
#define echoe 4
#define trigf 7
#define echof 6

const int BMI160_I2C_ADDRESS = 0x68;
#define INTERRUPT_PIN 0 
unsigned long int ultimamed;

#define botao_reset 10   

void stepInterrupt() {
  
}

double veloSom=344.0; //para temperatura de 20 graus 
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
  
  int16_t rawTemp=BMI160.readTemperature();
  float tempC = 23.0 + ((float)rawTemp / 512.0);

  veloSom = 331.3 + (0.606 * tempC);
  distance = ((double)duration*veloSom)/(2000000.0f); 
  return distance;
}

void setup() {
  Serial1.begin(9600,SERIAL_8N1,21,1); 

  while (!Serial1) { } 

  pinMode(botao_reset, INPUT_PULLDOWN);
  pinMode(trigd, OUTPUT); 
  pinMode(echod, INPUT);
  pinMode(trige, OUTPUT); 
  pinMode(echoe, INPUT);
  pinMode(trigf, OUTPUT); 
  pinMode(echof, INPUT);

  if (!BMI160.begin(BMI160GenClass::I2C_MODE, BMI160_I2C_ADDRESS, INTERRUPT_PIN)) {
    Serial.println("BMI160 initialization failed!");
    while (1);
  }
  BMI160.attachInterrupt(stepInterrupt);

  BMI160.autoCalibrateGyroOffset();
  BMI160.autoCalibrateXAccelOffset(0); // X-axis to 0g
  BMI160.autoCalibrateYAccelOffset(0); // Y-axis to 0g
  BMI160.autoCalibrateZAccelOffset(1); // Z-axis to +1g (gravity)
  
  BMI160.setFullScaleAccelRange(BMI160_ACCEL_RANGE_2G);
  BMI160.setIntFreefallEnabled(true);

  BMI160.setStepDetectionMode(BMI160_STEP_MODE_NORMAL);
  BMI160.setStepCountEnabled(true);
  BMI160.resetStepCount();
  
  delay(1000);
  ultMed=millis();
}

void loop() {
  if(BMI160.getIntFreefallStatus()){
    Serial1.println("QUEDA");
    while(!digitalRead(botao_reset) == HIGH){};
  }
  if(digitalRead(botao_reset) == HIGH) {
    BMI160.resetStepCount();
  }
  if(millis()-ultMed>=200){
  uint16_t currentSteps = BMI160.getStepCount();
  int16_t rawTemp=BMI160.readTemperature();
  float tempC = 23.0 + ((float)rawTemp / 512.0); 
  double distd=medeDist(trigd,echod);
  double diste=medeDist(trige,echoe);
  double distf=medeDist(trigf,echof);
  Serial1.print("distancia: ");Serial1.print(distd);Serial1.print(", ");Serial1.print(diste);Serial1.print(", ");Serial1.print(distf);Serial1.print(", ");Serial1.print(currentSteps);Serial1.print(", ");Serial1.println(tempC);
  ultMed=millis();
}
}
