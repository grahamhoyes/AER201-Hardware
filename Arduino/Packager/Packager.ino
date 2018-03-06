#include <Wire.h>
#include <Stepper.h>
#include <AccelStepper.h>
#include <Servo.h>

// Pin definitions
#define IRPin A0 // IR reflection sensor
#define NEMAStepPin 2  // Output to A4988 Stepper Motor controller for spinning box
#define NEMADirPin 7 // Direction the nema should go

// 28BYJ Motor Pins
#define motorPin1 3
#define motorPin2 4
#define motorPin3 5
#define motorPin4 6

#define DCSpeedPin 9 // PWM signal for controlling DC motor speeds
#define microServoPin 10 // PWM signal for micro servo

// Setup the Micro Servo
Servo microServo;
int microServoPos = 0;

#define debugPin 13 // For the onboard LED
#define MSPin 14 // Microswitch on A0
#define IRSensorPin 15 // Reflective IR sensor pin on A1 (can make Analog if necessary this way)

// Setup the 28BYJ stepper
AccelStepper boxHoldingStepper(8, motorPin1, motorPin2, motorPin3, motorPin4);

// Directions for NEMA
#define CW 0
#define CCW 1

// Positions of bins for micro servo - exact values will need to change
#define POS0 0
#define POS1 45
#define POS2 90
#define POS3 135
#define POS4 180

// Useful variables
#define NEMASteps 800 // The NEMA actually has 200 steps/rev, but we're using a 4:1 gear ratio
int compartmentNum;

void setup() {
  pinMode(NEMAStepPin, OUTPUT); // Output to A4988 Stepper Motor controller for spinning box
  pinMode(NEMADirPin, OUTPUT); // Output to A4988 Stepper Motor controller for direction control
  pinMode(DCSpeedPin, OUTPUT); // PWM signal for controlling DC motor speeds
  pinMode(MSPin, INPUT);
  pinMode(IRSensorPin, INPUT);

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
}

void loop() {
//  if (digitalRead(8) == HIGH) {
//    digitalWrite(13, LOW); 
//  } else {
//    digitalWrite(13, HIGH);
//  }
}

// Going to want to increase the speed here, 45 deg rotation takes 5 seconds
void stepNEMA(int steps) {
  /* Step period of 50 ms corersponds to 6 RPM, can speed up later */
  for (int i = 0; i < steps; i++) {
    digitalWrite(NEMAStepPin, HIGH);
    delay(25);
    digitalWrite(NEMAStepPin, LOW);
    delay(25);  
  }
}

void rotateNEMA(float deg) {
  stepNEMA((int)NEMASteps/(360/deg));
}

void boxSetting(void) {
  digitalWrite(NEMADirPin, CCW); // Counter-clockwise
  while(digitalRead(MSPin) == LOW) {
    /* While the microswitch hasn't detected the box lid, step the NEMA 17. */
    stepNEMA(1);
  }

  boxHoldingStepper.moveTo(640); // Perform 10 revolutions to grab the box
  while (boxHoldingStepper.distanceToGo() > 0) {
    boxHoldingStepper.run(); // Perform 1 step
  }

  digitalWrite(NEMADirPin, CW); // Rotating clockwise now
  rotateNEMA(45.0); // Rotate the box 45 deg (1/8 of 200 step rotation)

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
  } else {
    Serial.print(char(x));
    for (count = 1; count < numBytes; count++) {
      x = Wire.read();
      Serial.print(char(x)); // Print the rest anyway  
    }  
    Serial.print("\n");
  }
}


