#include <SharpIR.h>

#define IR A0 // define signal pin
#define model 1080 // used 1080 because model GP2Y0A21YK0F is used
// Sharp IR code for Robojax.com
// ir: the pin where your sensor is attached
// model: an int that determines your sensor:  1080 for GP2Y0A21Y
//                                            20150 for GP2Y0A02Y
//                                            430 for GP2Y0A41SK   
/*
2 to 15 cm GP2Y0A51SK0F  use 1080
4 to 30 cm GP2Y0A41SK0F / GP2Y0AF30 series  use 430
10 to 80 cm GP2Y0A21YK0F  use 1080
10 to 150 cm GP2Y0A60SZLF use 10150
20 to 150 cm GP2Y0A02YK0F use 20150
100 to 550 cm GP2Y0A710K0F  use 100550

 */

int trigPin = 11;
int echoPin = 12;
int signalPin = 13;
long duration;

SharpIR SharpIR(IR, model);

void setup() {
    // Sharp IR code for Robojax.com
 Serial.begin(9600);
 pinMode(7, OUTPUT);
 digitalWrite(7, LOW);

 pinMode(trigPin, OUTPUT);
 pinMode(echoPin, INPUT);
 pinMode(signalPin, OUTPUT);
}

void loop() {
     activateSonarSensor();
     activateIRSensor();
}


void activateIRSensor(){
  unsigned long startTime=millis();  // takes the time before the loop on the library begins

  int dis=SharpIR.distance();  // this returns the distance to the object you're measuring

  Serial.print("Mean distance: ");  // returns it to the serial monitor
  Serial.println(dis);
  //Serial.println(analogRead(A0));
  unsigned long endTime=millis()-startTime;  // the following gives you the time taken to get the measurement

  if (dis < 60){
   digitalWrite(7, HIGH);
  }
  else{
   digitalWrite(7,LOW);
  }
}

void activateSonarSensor(){
  digitalWrite(signalPin, LOW);
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);

  if (duration < 10000){
    digitalWrite(signalPin, HIGH);
  }
  delay(250);
}
