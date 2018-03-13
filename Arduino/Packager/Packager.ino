/* TODO 
* IR reflection is on analog, need to calibrate
* NEMA makes a ton of noise when stopped. Use A2 as nEnable
*/

#include <Wire.h>
#include <Stepper.h>
#include <AccelStepper.h>
#include <Servo.h>
#include "IR.h"

// Pin definitions
#define NEMAStepPin 2  // Output to A4988 Stepper Motor controller for spinning box
#define NEMADirPin 7 // Direction the nema should go
#define NEMAnEnablePin A2 // Active low enable for NEMA

// 28BYJ Motor Pins
#define motorPin1 3
#define motorPin2 4
#define motorPin3 5
#define motorPin4 6

#define DCSpeedPin 9 // PWM signal for controlling DC motor speeds
int DCSpeed = 255;
#define microServoPin 10 // PWM signal for micro servo

// Setup the Micro Servo
Servo microServo;
int microServoPos = 0;

#define debugPin 13 // For the onboard LED
#define MSPin A0 // Microswitch on A0. Not using anymore 
#define IRSensorPin A1 // Reflective IR sensor pin on A1 (can make Analog if necessary this way)

// Setup the 28BYJ stepper
AccelStepper boxHoldingStepper(8, motorPin1, motorPin2, motorPin3, motorPin4);

// Directions for NEMA
#define CW 0
#define CCW 1

// Positions of bins for micro servo - exact values will need to change
#define POS0 163 // Default position
#define POS1 0
#define POS2 30
#define POS3 60
#define POS4 90

// Useful variables
#define NEMASteps 2880 // NEMA has 200 steps/rev, 1/4 microstepping, 1:3.6 gearing. 200*4*3.6 = 2880
int compartmentNum;
int servoCurrPos = 0;

// IR Remote Debugging variables
#define IRRemotePin 8
IRrecv receiver(IRRemotePin);
decode_results output;

void setup() {
  pinMode(NEMAStepPin, OUTPUT); // Output to DRV8825 Stepper Motor controller for spinning box
  pinMode(NEMADirPin, OUTPUT); // Output to DRV8825 Stepper Motor controller for direction control
  pinMode(NEMAnEnablePin, OUTPUT); // Output to DRV8825 Stepper Motor controller for active low enable
  pinMode(DCSpeedPin, OUTPUT); // PWM signal for controlling DC motor speeds
  pinMode(MSPin, INPUT);
  pinMode(IRSensorPin, INPUT);

  digitalWrite(NEMAnEnablePin, HIGH); // Disable the stepper initially

  // 28BYJ Setup
  boxHoldingStepper.setMaxSpeed(100.0);
  boxHoldingStepper.setAcceleration(100.0); // Set the max acceleration to 100 steps/sec^2
  boxHoldingStepper.setSpeed(64.0); // Set the speed to 1 rev/second (64 steps/rev)

  // Servo setup
  microServo.attach(microServoPin);
  
  Wire.begin(8);
  Wire.onReceive(receiveEvent);

  Serial.begin(9600); // For communication with PC
  Serial.println("We're live");

  analogWrite(DCSpeedPin, DCSpeed);

  receiver.enableIRIn();
}

void stepNEMA(int steps) {
  digitalWrite(NEMAnEnablePin, LOW); // Enable the motor controller
  for (int i = 0; i < steps; i++) {
    digitalWrite(NEMAStepPin, HIGH);
    delay(1);
    digitalWrite(NEMAStepPin, LOW);
    delay(1);  
  }
  digitalWrite(NEMAnEnablePin, HIGH); // Disable the motor controller
}

void rotateNEMA(float deg) {
  stepNEMA((int)NEMASteps/(360/deg));
}

void boxSetting(void) {
  digitalWrite(NEMADirPin, CCW); // Counter-clockwise
//  while(digitalRead(MSPin) == LOW) {
//    /* While the microswitch hasn't detected the box lid, step the NEMA 17. */
//    stepNEMA(1);
//  }

//  boxHoldingStepper.moveTo(640); // Perform 10 revolutions to grab the box
//  while (boxHoldingStepper.distanceToGo() > 0) {
//    boxHoldingStepper.run(); // Perform 1 step
//  }

  rotateNEMA(720.0);
  digitalWrite(NEMADirPin, CW); // Rotating clockwise now
  rotateNEMA(45.0); // Rotate the box 45 deg

  while (digitalRead(IRSensorPin) == HIGH) { // Reflective IR goes low when triggered
    /* Step the NEMA until the C1 white strip is detected */
    stepNEMA(1);
  }

  compartmentNum = 8;
}

void boxUnsetting(void) {
  boxHoldingStepper.moveTo(0); // Return to initial holding position
  while (boxHoldingStepper.distanceToGo() > 0) {
    boxHoldingStepper.run(); // Perform 1 step  
  }

  microServo.write(POS0); // Reset micro servo to initial position
}

void setServoPos(int pos) {
  /* Moves the servo to pos, slowly */
  if (pos > servoCurrPos) {
    for (int i = servoCurrPos; i < pos; i++) {
      microServo.write(i);
      delay(10);  
    }  
  } else if (pos < servoCurrPos) {
    for (int i = servoCurrPos; i > pos; i--) {
      microServo.write(i);
      delay(10);  
    } 
  }

  servoCurrPos = pos;
}

void receiveEvent(int numBytes) {
  int count;
  uint8_t x;

  x = Wire.read();

  if (x == 1) {     // Debugging message: Print rest of message to serial
    for (count = 1; count < numBytes; count++) {
      x = Wire.read();
      Serial.print((char)x); // Print ASCII representation of byte to serial monitor
    }
    Serial.print("\n");
  } else if (x == 2) { // Instruction to rotate the NEMA17 45 deg CW
    digitalWrite(NEMADirPin, CW);
    rotateNEMA(45.0);
    Serial.println("Box stepped");
    Serial.println("Rotated the stepper");
  } else if (x == 3) { // Instruction to rotate the NEMA17 359.5 deg CW and step Micro Servo over first bin
    digitalWrite(NEMADirPin, CW);
    rotateNEMA(359.5); // This will get truncated down to 359 deg in reality
    Serial.println("Rotated the stepper");
    microServo.write(POS1);
  } else if (x == 4) { // Instruction to step micro servo over second dispensing bin
    microServo.write(POS2);
    Serial.println("Servo over bin 2");
  } else if (x == 5) { // Instruction to step micro servo over third dispensing bin
    microServo.write(POS3);  
    Serial.println("Servo over bin 3");
  } else if (x == 6) { // Instruction to step micro servo over fourth dispensing bin
    microServo.write(POS4);  
    Serial.println("Servo over bin 4");
  } else if (x == 7) { // Instruction to release the box and reset the micro servo
    boxUnsetting();
    Serial.println("Box removed");
  } else if (x == 8) { // Instruction to start box setting, because I forgot it earlier
    Serial.println("Box being set");
    boxSetting();
    Serial.println("Box setting complete");
  } else {
    Serial.print(char(x));
    for (count = 1; count < numBytes; count++) {
      x = Wire.read();
      Serial.print(char(x)); // Print the rest anyway  
    }  
    Serial.print("\n");
  }
}
int nemaDir = CW;
void loop() {
  if (receiver.decode(&output)) {
    unsigned int value = output.value;
    output.value = 0;
    switch(value) {
      case zero_key: // Reverse stepper direction
        Serial.println("Zero pressed: Stepper direction changed");
        nemaDir = (nemaDir == CW) ? CCW : CW;
        digitalWrite(NEMADirPin, nemaDir);
        break;
      case one_key: // Step the box 45 deg
        Serial.println("One pressed: Box spun 45 deg");
        rotateNEMA(45);
        break;
      case two_key: // Cycle the micro servo position
        Serial.println("Two pressed: Servo moved");
        if (servoCurrPos == POS0) {setServoPos(POS1);}
        else if (servoCurrPos == POS1) {setServoPos(POS2);}
        else if (servoCurrPos == POS2) {setServoPos(POS3);}
        else if (servoCurrPos == POS3) {setServoPos(POS4);}
        else if (servoCurrPos == POS4) {setServoPos(POS0);}
        else {setServoPos(POS0);}
        break;
      case voldown_key:
        Serial.print("Volume down pressed: Motors slowed down to ");
        DCSpeed = (DCSpeed >= 10) ? DCSpeed - 10 : 0;
        Serial.println(DCSpeed);
        analogWrite(DCSpeedPin, DCSpeed);
        break;
      case volup_key:
        Serial.print("Volume up pressed: Motors sped up to ");
        DCSpeed = (DCSpeed <= 245) ? DCSpeed + 10 : 255;
        Serial.println(DCSpeed);
        analogWrite(DCSpeedPin, DCSpeed);
    }
    //Serial.print("IR received: ");
    //Serial.print(value);
    //Serial.print("\n");
    value = 0;
    receiver.resume();
  }
}


