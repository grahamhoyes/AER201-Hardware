#include <Wire.h>

void setup() {
  Wire.begin(8);
  Wire.onReceive(receiveEvent);

  Serial.begin(9600); // For communication with PC
  Serial.println("We're live");

  pinMode(8, INPUT);
  pinMode(9, OUTPUT);
}

void loop() {
  if (digitalRead(8) == HIGH) {
    digitalWrite(9, LOW);
  } else {
    digitalWrite(9, HIGH);
  }
}

void receiveEvent(int numBytes) {
  int count;
  uint8_t x;
  for (count = 0; count < numBytes; count++) {
    x = Wire.read(); // Receive byte as unsigned char
    Serial.print((char)x); // Print ASCII representation of byte to serial monitor  
  }
  Serial.print("\n");
}


