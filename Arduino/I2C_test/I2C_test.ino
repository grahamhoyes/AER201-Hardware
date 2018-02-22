#include <Wire.h>

void setup() {
  Wire.begin(8);
  Wire.onReceive(receiveEvent);

  Serial.begin(9600); // For communication with PC
  Serial.println("We're live");
}

void loop() {
  
}

void receiveEvent(int numBytes) {
  int count;
  uint8_t x;
  Serial.println(numBytes);
  for (count = 0; count < numBytes; count++) {
    x = Wire.read(); // Receive byte as unsigned char
    Serial.print((char)x); // Print ASCII representation of byte to serial monitor  
  }
  Serial.print("\n");
}
