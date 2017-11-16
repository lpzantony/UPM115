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

int           speed             = 10;
const uint8_t MINSPEED          = 128; //128
int           DISCRETE_MV_TIME  = 6; //discrete time shared between ON and OFF mode for motors. (PWM squared)

//##############################//
//     Ultrasonic constants     //
//##############################//
const uint8_t ECHO = 5;
const uint8_t TRIG = 3;
const uint8_t SERVO = 4;

const int NB_ANGLES   = 44;
const int MAXRIGHT    = 30;
const int MAXLEFT     = 250;
const int MID         = (MAXLEFT - MAXRIGHT) / 2;
const int STEP_SIZE   = (MAXLEFT - MAXRIGHT) / NB_ANGLES;

uint8_t angles[NB_ANGLES]; //Stores the init scan values


//##############################//
//            Objects           //
//##############################//

// Motors driver
L298N driver(ENA, IN1, IN2, IN3, IN4, ENB, false, MINSPEED, DISCRETE_MV_TIME);

// Distance sensor
Ultrasonic ultrasonic(TRIG, ECHO); // (Trig PIN,Echo PIN)

// Distance sensor mount motor
Servo myservo;  // create servo object to control a servo

// FSM states and variables
enum State {IDLE, FORWARD_RIGHT, FORWARD_LEFT, FORWARD, CLOSE_ENOUGH};
enum SensorState {SIDLE, LOOKING_RIGHT, LOOKING_LEFT};

State state = IDLE;
SensorState sensor_state = SIDLE;
uint8_t sensor_angle = 0;
uint8_t target_dist = 0;
uint8_t prevprev_dist = 0;
uint8_t prev_dist = 0;
uint8_t curr_dist = 0;


//##############################//
//         Setup function       //
//##############################//
void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);
    myservo.attach(SERVO);  // attaches the servo on pin 9 to the servo object
    myservo.write(MAXRIGHT);   // 255 max left, 30 min right, 109 mid
    delay(1000);
    int angle = MAXRIGHT;
    for (int i = 0; i < NB_ANGLES; i++) angles[i] = 255;
    for (int i = 0; i < NB_ANGLES; i++) {
        angles[i] = ultrasonic.Ranging(CM);
        angle += STEP_SIZE;
        myservo.write(angle);
        delay(100);

    }
    char str[4] = "";
    for (int i = 0; i < NB_ANGLES ; i++) {
        sprintf(str, "%-3d", i);
        Serial.print(String(str) + " ");
    }
    Serial.println();
    for (int i = MAXRIGHT; i < MAXLEFT ; i += STEP_SIZE) {
        sprintf(str, "%-3d", i);
        Serial.print(String(str) + " ");
    }
    Serial.println();
    for (int i = 0; i < NB_ANGLES ; i++) {
        sprintf(str, "%-3d", angles[i]);
        Serial.print(String(str) + " ");
    }
    Serial.println();
    Serial.println("Hello");

    uint8_t min  = 255;
    uint8_t max  = 0;
    uint8_t min_index  = NB_ANGLES + 1;
    for (int i = 0; i < NB_ANGLES; i++) {
        if (angles[i] < min) {
            min  = angles[i];
            min_index = i;
        }
        if (angles[i] > max) {
            max  = angles[i];
        }
    }
    target_dist = min;
    if (min_index >= NB_ANGLES + 1) {
        state = FORWARD;
        sensor_state = SIDLE;
        target_dist = max;
    }
    else if (min_index < NB_ANGLES / 2) {
        state = FORWARD_RIGHT;
        sensor_state = LOOKING_RIGHT;
    }
    else if (min_index > NB_ANGLES / 2) {
        state = FORWARD_LEFT;
        sensor_state = LOOKING_LEFT;
    }
    else {
        state = FORWARD;
        sensor_state = SIDLE;
    }
    speed = 30;
    myservo.write(MID);
    delay(400);
    prev_dist = min;
    curr_dist = ultrasonic.Ranging(CM);
    prev_dist = curr_dist;
    prevprev_dist = curr_dist;
    sensor_angle = angles[min_index];
}



//##############################//
//         Loop function       //
//##############################//
int cdelay = 500;
int ms_diff = 70;
int sensor_angle_step = 20;
uint8_t cpt_stab = 0;
const uint8_t cpt_stab_lim = 3;
void loop() {
    /*
        Serial.print(ultrasonic.Ranging(CM)); // CM or INC
        Serial.println(" cm" );
        delay(500);
    */

    //=======> Sensor motor managements

    curr_dist = ultrasonic.Ranging(CM);

    switch (sensor_state) {
        case LOOKING_RIGHT:
            Serial.println("Going right.    prev2 = "+ String(prevprev_dist)+" prev = " + String(prev_dist) + " curr = " + String(curr_dist) + " angle = " + String(sensor_angle));
            if (prev_dist < curr_dist && prevprev_dist < prev_dist) {
                sensor_state  = LOOKING_LEFT;
            } else if (prev_dist == curr_dist) {
                //sensor_state  = SIDLE;
                cpt_stab++;
            }
            else {
                sensor_angle -= sensor_angle_step;
                if (sensor_angle > MAXLEFT)
                    sensor_angle = MAXLEFT;
            }
            break;
        case LOOKING_LEFT:
            Serial.println("Going left.     prev2 = "+ String(prevprev_dist)+" prev = " + String(prev_dist) + " curr = " + String(curr_dist) + " angle = " + String(sensor_angle));
            if (prev_dist < curr_dist && prevprev_dist < prev_dist) {
                sensor_state  = LOOKING_RIGHT;
            } else if (prev_dist == curr_dist) {
                //sensor_state  = SIDLE;
                cpt_stab++;
            }
            else {
                sensor_angle += sensor_angle_step;
                if (sensor_angle < MAXRIGHT)
                    sensor_angle = MAXRIGHT;
            }
            break;
        case SIDLE:
            if (prev_dist != curr_dist)
                sensor_state  = LOOKING_RIGHT;
            Serial.println("Going no where. prev2 = "+ String(prevprev_dist)+" prev = " + String(prev_dist) + " curr = " + String(curr_dist) + " angle = " + String(sensor_angle));
            break;
    }
    if (cpt_stab >= cpt_stab_lim) {
        cpt_stab = 0;
        sensor_state = SIDLE;
    }
    myservo.write(sensor_angle);

    delay(50);
    prevprev_dist  = prev_dist;
    prev_dist = curr_dist;

    //=======> Motors managements

    switch (state) {
        case IDLE:
            //driver.stop();
            //delay(cdelay);
            break;
        case FORWARD:
            //driver.cdrive(L298N::FORWARD, speed, cdelay);
            break;
        case FORWARD_LEFT:
            //driver.cdrive(L298N::FORWARD_L, speed, cdelay, ms_diff);
            break;
        case FORWARD_RIGHT:
            //driver.cdrive(L298N::FORWARD, speed, cdelay, ms_diff);
            break;
        case CLOSE_ENOUGH:
            //driver.stop();
            //delay(cdelay);
            break;
    }

}



