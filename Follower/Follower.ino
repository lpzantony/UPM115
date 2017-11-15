#include <Servo.h>
#include "Ultrasonic.h"
#include "L298N.h"

//##############################//
//       Motors constants       //
//##############################//
const uint8_t ENA = 12;
const uint8_t ENB = 7;
const uint8_t IN1 = 11;
const uint8_t IN2 = 10;
const uint8_t IN3 = 9;
const uint8_t IN4 = 8;

const uint8_t MINSPEED = 128; //128
const uint8_t MAXSPEED = 255;
int           delay_time = 1000;

//##############################//
//     Ultrasonic constants     //
//##############################//
const uint8_t ECHO = 5;
const uint8_t TRIG = 3;
const uint8_t SERVO = 2;

const int NB_ANGLES   = 44;
const int MAXRIGHT    = 30;
const int MAXLEFT     = 250;
const int MID         = (MAXLEFT - MAXRIGHT) / 2;
const int STEP_SIZE = (MAXLEFT - MAXRIGHT) / NB_ANGLES;

uint8_t angles[NB_ANGLES];

L298N driver(ENA, IN1, IN2, IN3, IN4, ENB, false, MINSPEED); 
Ultrasonic ultrasonic(TRIG,ECHO); // (Trig PIN,Echo PIN)
Servo myservo;  // create servo object to control a servo

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  myservo.attach(SERVO);  // attaches the servo on pin 9 to the servo object
  myservo.write(MAXRIGHT);   // 255 max left, 30 min right, 109 mid
  delay(1000);
  int angle = MAXRIGHT;
  for(int i=0; i< NB_ANGLES; i++) angles[i] = 255;
  for(int i=0; i< NB_ANGLES; i++){
    angles[i] = ultrasonic.Ranging(CM);
    angle += STEP_SIZE;
    myservo.write(angle);
    delay(100);
    
  }
  char str[4] ="";
  for(int i = 0; i<NB_ANGLES ; i++){
    sprintf(str,"%-3d",i);
    Serial.print(String(str) + " ");
  }
  Serial.println();
  for(int i = MAXRIGHT; i<MAXLEFT ; i+= STEP_SIZE){
    sprintf(str,"%-3d",i);
    Serial.print(String(str) + " ");
  }
  Serial.println();
  for(int i = 0; i<NB_ANGLES ; i++){
    sprintf(str,"%-3d",angles[i]);
    Serial.print(String(str) + " ");
  }
  Serial.println();
  Serial.println("Hello");
  delay(1000);
  
}

void loop() {
  //Serial.print(ultrasonic.Ranging(CM)); // CM or INC
  //Serial.println(" cm" );
  driver.cdrive(driver.FORWARD, 110,100, 2000);

  driver.stop();
  delay(1000); 



}



