#	Tracker : your personal robot follower

The project consists in a robot capable of moving in a 2D environment. It can be remotely controlled by an user, but its main goal is to follow a target, through a sonar sensor.

##	Building the robot

We use the KeyesL298 card to control the motors, the idea is to deliever much more power than what you would normally be able to deliever through the Arduino Nano card. Moreover, it is really simple to use, because a lot of librairies exist, and they are all ready-to-use. For instance, in order to make the motor go forward, you just do

```C++
/*#########################
##     MOTOR CONSTANTS   ##
#########################*/

const uint8_t ENA = 12;
const uint8_t ENB = 7;
const uint8_t IN1 = 11;
const uint8_t IN2 = 10;
const uint8_t IN3 = 9;
const uint8_t IN4 = 8;

const uint8_t MINSPEED = 128;

L298N driver(ENA, IN1, IN2, IN3, IN4, ENB, false, MINSPEED);
driver.forward(SPEED,DURATION);
```

Which is easier than calling for the pins directly.

![](https://markdown.data-ensta.fr/uploads/upload_3ee848d7648504ebaedca27572d5b709.jpg)

In order to detect our environment, we use a sonar sensor at the front of the robot, mounted on a servo motor, to be able to detect obstacles in any direction.
![](https://markdown.data-ensta.fr/uploads/upload_4b2434f57d0142fc3f23ddf196c73b06.jpg)

You send an ultrasonic wave, waits for it to bounce on an obstacle,and then sense the reflection. The intensity of the reflection indicates the proximity of the obstacle. Our sensors were not really precise, so we were not able to detect obstacles from more than 20 centimeters, but the system gave really good results within this distance.


##	Sensor & Following

What to do once you have the sensors information, and that you are able to move freely ? The answer is : follow ! We decided to only modify the direction of the robot, which was related to the difference between the speed of the two motors.

$$
\omega_2 - \omega_1 = f(I_{sensor}(t),t)
$$

We decided to use a control system, that use the intensity of the captor as the control variable over the difference of speed.

The basic idea is that when $\frac{dI_{sensor}}{dt}>0$, when continue to go in the same direction. However, when $\frac{dI_{sensor}}{dt}<0$, on change de direction.

With this method, we can follow the target pretty efficiently.

##	Controlling through Wi-Fi