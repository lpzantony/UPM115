# Tracker : your personal robot follower
### *Antoine Gauthier, Antony Lopez*

The project consists in a robot abble to follow a moving object on a flat surface.
It can be remotely controlled by an user using WiFi, but its main goal is to follow a target, using a sonar sensor.

## Building the robot

We use the KeyesL298 board to control the motors. The idea is to deliver much more power to the motors than what would
 normally be able to deliever the Arduino Nano board. Also the KeyesL298 gives us a +5V pin regulated from the 9V battery
to power the arduino.
Moreover, KeyesL298 is a really easy to use board, thanks to all the librairies which already exist. 
For instance, using the XXX_TO_COMPLETE_XXX library, in order to make the motor go forward you just use the following code:

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

Which is easier than managing directly the pins.

### Picture of the robot without the WiFi module

![](https://markdown.data-ensta.fr/uploads/upload_3ee848d7648504ebaedca27572d5b709.jpg)

### Ultrasonic Module SR04
![](https://markdown.data-ensta.fr/uploads/upload_4b2434f57d0142fc3f23ddf196c73b06.jpg)

In order to detect the environment, we use a sonar sensor at the front of the robot,
 mounted on a servo motor. This way, we are able to detect obstacles in any direction in front of the robot.

The distance estimation process is quite simple, you send an ultrasonic wave, 
waits for it to bounce on an obstacle, and then detect the reflection. 
The time it took for the ultrasonic wave to go back and forth let us deduce the distance of the obstacle. 
We were able to detect obstacles from at most 50 centimeters and 
the system finally could give good results when we applied a basic average filter to get
rid of some random uncoherent values.

### Motors

The very first issue we had to face was the motors's speed. Indeed, below a PWM duty cycle of 50%, we could not make it move at all.
The solution we used is kind of original, we applied a PWM behaviour to our calls to the PWM Arduino functions.

To be more specific, we chose a discrete amount of time (6ms) and divided this time in two phases, PWM at 100% and PWM at 0%.
This way we were abble to maintain a pretty smooth speed on the motors.
For example, a smooth "low" speed could be achieved spending 2ms at 100% and 4ms at 0%.

To implement this "software" PWM we based our code from the `drive()` function of the KeyesL298 library.

## Sensor & Following

What can you do once you have the sensors information and two fully functionnal motors ? The answer is : follow ! 
We decided to only modify the direction of the robot, which was related to the difference between the speed of the two motors.

$$
\omega_2 - \omega_1 = f(I_{sensor}(t),t)
$$

We decided to use a control system, that use the intensity of the captor as the control variable over the difference of speed.

The basic idea is that when 
$\frac{dI_{sensor}}{dt}>0$, we continue to go in the same direction. However, when $\frac{dI_{sensor}}{dt}<0$, we change the direction.

With this method, we can follow the target pretty efficiently.

##	Controlling through Wi-Fi

![](https://markdown.data-ensta.fr/uploads/upload_b49a26ba9f2de3734bb05b6eb458b87c.png)

We use the ESP8266 Wifi component to interface the arduino and the wifi. The component is controlled through a SoftwareSerial object in the arduino code. We use a library specifically designed to control this card to manipulate Wi-Fi. 

We then create a TCP server on the Process application, that waits for the client to connect. Once the robot is connected, we can send orders to the robots through packets containing the input.

```C++
#include "ESP8266.h"

#define SSID        "Athens2016"
#define PASSWORD    "Arduino2016"
#define HOST_NAME   "192.168.0.129"
#define HOST_PORT   22522

SoftwareSerial mySerial(6,7);

ESP8266 wifi(mySerial);

void setup(void)
{
    Serial.begin(9600);
    Serial.print("setup begin\r\n");
    
    Serial.print("FW Version:");
    Serial.println(wifi.getVersion().c_str());
      
    if (wifi.setOprToStation()) {
        Serial.print("to station + softap ok\r\n");
    } else {
        Serial.print("to station + softap err\r\n");
    }
 
    if (wifi.joinAP(SSID, PASSWORD)) {
        Serial.print("Join AP success\r\n");
        Serial.print("IP:");
        Serial.println( wifi.getLocalIP().c_str());       
    } else {
        Serial.print("Join AP failure\r\n");
    }
    
    if (wifi.disableMUX()) {
        Serial.print("single ok\r\n");
    } else {
        Serial.print("single err\r\n");
    }
    
    Serial.print("setup end\r\n");
}
 
void loop(void)
{
    uint8_t buffer[128] = {0};
    
    if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
        Serial.print("Connected to server \n");
    } else {
        Serial.print("Disconnected from server \n");
    }
    
    uint32_t len = wifi.recv(buffer, sizeof(buffer), 10000);
    if (len > 0) {
        Serial.print("Received:[");
        for(uint32_t i = 0; i < len; i++) {
            Serial.println((int)buffer[i]);//Le buffer contient la valeur : 1 = z / 2 = q / 3 = s /4 = d
        }
        Serial.print("]\r\n");
    }
}
     
```

##	Process App

The interface is basic, we just use a graphical way of seeing the input we enter. Every time the user hits a key, we get it, and if it is directional keys or the spacebar, we change the packet value, from 0 to a specified value for each key.

![](https://markdown.data-ensta.fr/uploads/upload_4fdb2260e80003bd0984da7f7d26562b.png)


During this time, the server send packets all the time, sending its state. It allows the user to control remotely the robot. 

##	Problems we encountered

We had trouble controlling the robot, since our component was not working really well, so as we had not enough time, we decided to skip the wifi part, and just focus on the sensor. Our algorithm would not function as expected, so we decided to build something else : we changed our algorithm, and as we already had a robot structure that was functioning, we decided to do a robot that avoided walls and obstacle.

![](https://markdown.data-ensta.fr/uploads/upload_ae04fb40696392c4d8f46b5d5c87f1ec.jpg) 