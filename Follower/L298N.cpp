// ---------------------------------------------------------------------------
// Created by Alonso José Lara Plana - alonso.lara.plana@gmail.com
// Copyright 2016 License: GNU GPL v3 http://www.gnu.org/licenses/gpl.html
//
// See "L298N.h" for purpose, syntax, version history, links, and more.
// ---------------------------------------------------------------------------

#include "L298N.h"

uint8_t ENA, ENB, IN1, IN2, IN3, IN4;
boolean INVERT = false;
uint8_t MINSPEED = 0;



// ---------------------------------------------------------------------------
// L298N constructor
// ---------------------------------------------------------------------------

L298N::L298N(uint8_t ena, uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4, uint8_t enb, boolean invert, uint8_t minspeed, uint8_t DISCRETE_MV_TIME)
{
    pinMode (ena, OUTPUT);
    pinMode (in1, OUTPUT);
    pinMode (in2, OUTPUT);
    pinMode (in3, OUTPUT);
    pinMode (in4, OUTPUT);
    pinMode (enb, OUTPUT);

    ENA = ena;
    ENB = enb;

    IN1 = in1;
    IN2 = in2;
    IN3 = in3;
    IN4 = in4;

    INVERT = invert;
    MINSPEED = minspeed;
    this->DISCRETE_MV_TIME = DISCRETE_MV_TIME;
}


// ---------------------------------------------------------------------------
// L298N Complex Method
// ---------------------------------------------------------------------------

void L298N::cdrive(uint8_t direction, uint8_t speed, int delay_time, uint8_t ms_diff)
{
    if ( direction == FORWARD  || direction == FORWARD_R  || direction == FORWARD_L  || \
            direction == BACKWARD || direction == BACKWARD_R || direction == BACKWARD_L || \
            direction == RIGHT    || direction == LEFT       || \
            direction == STOP     || direction == BRAKE )
    {
        if (speed > 100) speed = 100;
        digitalWrite(IN1, bitRead(direction, 3));
        digitalWrite(IN2, bitRead(direction, 2));
        digitalWrite(IN3, bitRead(direction, 1));
        digitalWrite(IN4, bitRead(direction, 0));

        int onTime = DISCRETE_MV_TIME * (float(speed) / 100);
        int offTime = DISCRETE_MV_TIME  - onTime;

        //if moving with a tilt
        if ( direction == FORWARD_R  || direction == FORWARD_L || direction == BACKWARD_R || direction == BACKWARD_L) {

            // ms_diff variable represents the amount
            //  of time (in percent) spent at STOCK_SPEED by the slower wheel
            //  compared to the faster wheel. Setting this variable
            // to 100 will have the same behaviour than FORWARD/BACKWARD
            if (ms_diff > 100) ms_diff = 100;
            int slaveOnTime = onTime * (float(ms_diff) / 100);

            // Generating a PWM behavior on the PWM output
            for (int i = 0; i < delay_time; i += DISCRETE_MV_TIME) {
                analogWrite((direction == FORWARD_R || direction == BACKWARD_R) ? ENB : ENA, STOCK_SPEED);
                delay(onTime - slaveOnTime);
                analogWrite((direction == FORWARD_R || direction == BACKWARD_R) ? ENA : ENB, STOCK_SPEED);
                delay(slaveOnTime);
                analogWrite(ENB, 0);
                analogWrite(ENA, 0);
                delay(offTime);
            }
        }
        //if moving on itself
        else if (direction == RIGHT || direction == LEFT ) {
            analogWrite((direction == RIGHT) ? ENA : ENB, 0);
            for (int i = 0; i < delay_time; i += DISCRETE_MV_TIME) {
                analogWrite((direction == RIGHT) ? ENA : ENB, STOCK_SPEED);
                delay(onTime);
                analogWrite((direction == RIGHT) ? ENA : ENB, 0);
                delay(offTime);
            }
        }
        //if moving in a straight line
        else {
            for (int i = 0; i < delay_time; i += DISCRETE_MV_TIME) {
                analogWrite(ENA, STOCK_SPEED);
                analogWrite(ENB, STOCK_SPEED);
                delay(onTime);
                analogWrite(ENA, 0);
                analogWrite(ENB, 0);
                delay(offTime);
            }
        }
    }

}

void L298N::drive(uint8_t direction, uint8_t speed, uint8_t slave_ratio, int delay_time)
{
    if ( direction == FORWARD  || direction == FORWARD_R  || direction == FORWARD_L  || \
            direction == BACKWARD || direction == BACKWARD_R || direction == BACKWARD_L || \
            direction == RIGHT    || direction == LEFT       || \
            direction == STOP     || direction == BRAKE )
    {
        uint8_t master = 255, slave = 0;

        // MINSPEED <= speed_master <= MAXSPEED (255) || 255 if BRAKE
        if (direction != BRAKE)
            master = speed < MINSPEED ? MINSPEED : speed;

        // 0 <= speed_slave <= speed*slave_ratio/100 || speed_slave=speed if slave_ratio==100
        if (direction != STOP)
            slave = slave_ratio == 100 ? speed : (speed <= MINSPEED ? 0 : speed * slave_ratio / 100);

        digitalWrite(IN1, bitRead(direction, INVERT ? 1 : 3));
        digitalWrite(IN2, bitRead(direction, INVERT ? 0 : 2));
        digitalWrite(IN3, bitRead(direction, INVERT ? 3 : 1));
        digitalWrite(IN4, bitRead(direction, INVERT ? 2 : 0));

        analogWrite(ENA, bitRead(direction, INVERT ? 4 : 5) ? master : slave);
        analogWrite(ENB, bitRead(direction, INVERT ? 5 : 4) ? master : slave);

        delay(delay_time);
    }
}


// ---------------------------------------------------------------------------
// L298N Simple Methods
// ---------------------------------------------------------------------------

void L298N::stop(boolean brake, int delay_time)
{
    this->drive(brake ? BRAKE : STOP, 0, 0, delay_time);
}

void L298N::forward(uint8_t speed, int delay_time)
{
    this->drive(FORWARD, speed, 100, delay_time);
}

void L298N::backward(uint8_t speed, int delay_time)
{
    this->drive(BACKWARD, speed, 100, delay_time);
}

void L298N::left(uint8_t speed, int delay_time)
{
    this->drive(LEFT, speed, 100, delay_time);
}

void L298N::right(uint8_t speed, int delay_time)
{
    this->drive(RIGHT, speed, 100, delay_time);
}
