/* TODO 
* IR reflection is on analog, need to calibrate
* DCSpeedControl doesn't properly dim an LED, does weird stuff with servo when on D10
*   Either Cam or Danial had some kind of issue like this, talk to them
* Servo library disables PWM on 9 and 10 (servo on 10 still fine?)
*/

#include <Wire.h>
#include <Servo.h>
#include "IR.h"

// Pin definitions
#define NEMAStepPin 2  // Output to A4988 Stepper Motor controller for spinning box
#define NEMADirPin 7 // Direction the nema should go
#define NEMAnEnablePin A2 // Active low enable for NEMA

#define nDonePin A3 // Low when the instruction completes

#define DCSpeedPin 5 // PWM signal for controlling DC motor speeds
//int DCSpeed = 102;
int DCSpeed = 140;
#define microServoPin 12 // PWM signal for micro servo

// Setup the Micro Servo
Servo microServo;
int microServoPos = 0;

#define debugPin 13 // For the onboard LED
#define MSPin A0 // Microswitch on A0. Not using anymore 
#define IRSensorPin A1 // Reflective IR sensor pin on A1 (can make Analog if necessary this way)
#define IRSensorThreshold 70 // Analog threshold for triggering things

// Directions for NEMA
#define CW 0
#define CCW 1

// Positions of bins for micro servo - exact values will need to change
#define POS0 165 // Default position
#define POS1 0
#define POS2 30
#define POS3 70
#define POS4 110

// Useful variables
#define NEMASteps 2880 // NEMA has 200 steps/rev, 1/4 microstepping, 1:3.6 gearing. 200*4*3.6 = 2880
//#define NEMASteps 1440
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
  pinMode(nDonePin, OUTPUT);
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

  digitalWrite(NEMAnEnablePin, HIGH); // Disable the stepper initially
  digitalWrite(nDonePin, HIGH);

  // Servo setup
  microServo.attach(microServoPin);
  
  Wire.begin(8);
  Wire.onReceive(receiveEvent);

  Serial.begin(9600); // For communication with PC
  Serial.println("We're live");

  analogWrite(DCSpeedPin, DCSpeed);

  receiver.enableIRIn();

  setServoPos(POS1);
}

void doneSignal(void) {
  digitalWrite(nDonePin, LOW);
  delay(500);
  digitalWrite(nDonePin, HIGH);  
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
  stepNEMA((int)(NEMASteps/(360/deg)));
}

void boxSetting(void) {
  digitalWrite(nDonePin, HIGH);
  digitalWrite(NEMADirPin, CW); // Counter-clockwise
//  while(digitalRead(MSPin) == LOW) {
//    /* While the microswitch hasn't detected the box lid, step the NEMA 17. */
//    stepNEMA(1);
//  }

//  boxHoldingStepper.moveTo(640); // Perform 10 revolutions to grab the box
//  while (boxHoldingStepper.distanceToGo() > 0) {
//    boxHoldingStepper.run(); // Perform 1 step
//  }

  rotateNEMA(540.0);
  //digitalWrite(NEMADirPin, CW); // Rotating clockwise now
  //rotateNEMA(45.0); // Rotate the box 45 deg

  Serial.println("Finding compartment 1");
  int count = 0;
  digitalWrite(NEMAnEnablePin, LOW); // Enable the motor controller
  while (1) { // Reflective IR goes low when triggered
    /* Step the NEMA until the C1 white strip is detected */
    //Serial.println(analogRead(IRSensorPin));
    if (analogRead(IRSensorPin) >= IRSensorThreshold) {
      count++;
      if (count == 3) break; 
    } else {
      count = 0;  
    }
    digitalWrite(NEMAStepPin, HIGH);
    delay(2);
    digitalWrite(NEMAStepPin, LOW);
    delay(2);
  }
  digitalWrite(NEMAnEnablePin, HIGH); // Disable the motor controller
  stepNEMA(280); // Proper alignment over compartment 8?
  Serial.println("Compartment 1 found");
  compartmentNum = 8;
  doneSignal();
}

void boxUnsetting(void) {
  // Really this should start out of the way and move in at start
  setServoPos(POS0); // Reset micro servo to initial position
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
  doneSignal();
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
    Serial.println("Received 2");
    digitalWrite(NEMADirPin, CW);
    rotateNEMA(46);
    Serial.println("Box stepped");
    doneSignal();
  } else if (x == 3) { // Instruction to rotate the NEMA17 359.5 deg CW and step Micro Servo over first bin
    digitalWrite(NEMADirPin, CCW);
    rotateNEMA(540); // This will get truncated down to 359 deg in reality
    Serial.println("Rotated the stepper");
    setServoPos(POS1);
    doneSignal();
  } else if (x == 4) { // Instruction to step micro servo over second dispensing bin
    setServoPos(POS2);
    Serial.println("Servo over bin 2");
    doneSignal();
  } else if (x == 5) { // Instruction to step micro servo over third dispensing bin
    setServoPos(POS3);  
    Serial.println("Servo over bin 3");
    doneSignal();
  } else if (x == 6) { // Instruction to step micro servo over fourth dispensing bin
    setServoPos(POS4);  
    Serial.println("Servo over bin 4");
    doneSignal();
  } else if (x == 7) { // Instruction to release the box and reset the micro servo
    boxUnsetting();
    Serial.println("Box removed");
  } else if (x == 8) { // Instruction to start box setting, because I forgot it earlier
    Serial.println("Box being set");
    setServoPos(POS0);
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
long last = millis();
int stepsMade = 0;
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
        rotateNEMA(45.3);
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
        break;
      case four_key:
        stepNEMA(1);
        stepsMade = (nemaDir == CW) ? stepsMade + 1 : stepsMade - 1;
        Serial.print("Nema stepped 1 step. Total ");
        Serial.println(stepsMade);
        break;
      case five_key:
        stepNEMA(5);
        stepsMade = (nemaDir == CW) ? stepsMade + 5 : stepsMade - 5;
        Serial.print("Nema stepped 5 steps. Total ");
        Serial.println(stepsMade);
        break;
      case six_key:
        stepNEMA(10);
        stepsMade = (nemaDir == CW) ? stepsMade + 10 : stepsMade - 10;
        Serial.print("Nema stepped 10 steps. Total ");
        Serial.println(stepsMade);
        break;
      case seven_key:
        stepNEMA(50);
        stepsMade = (nemaDir == CW) ? stepsMade + 50 : stepsMade - 50;
        Serial.print("Nema stepped 50 steps. Total ");
        Serial.println(stepsMade);
        break;
      case eight_key:
        stepNEMA(100);
        stepsMade = (nemaDir == CW) ? stepsMade + 100 : stepsMade - 100;
        Serial.print("Nema stepped 100 steps. Total ");
        Serial.println(stepsMade);
        break;
      case nine_key:
        stepNEMA(500);
        stepsMade = (nemaDir == CW) ? stepsMade + 500 : stepsMade - 500;
        Serial.print("Nema stepped 500 steps. Total ");
        Serial.println(stepsMade);
        break;
      case play_key:
        stepsMade=0;
        Serial.println("Steps counter reset");
        break;
      case ch_key:
        boxSetting();
        break;
    }
    value = 0;
    receiver.resume();
  }
  /*
  if (millis() > last + 2000) {
    Serial.print("Analog IR Sensor: ");
    Serial.println(analogRead(IRSensorPin));
    last = millis();
  }*/
}


