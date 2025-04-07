#include <Wire.h>
#include <MT6701.h>

MT6701 encoder;

void setup() {
  Wire1.setSDA(2);
  Wire1.setSCL(3);
  Wire1.begin();
  encoder.initializeI2C();
  Serial.begin(115200);

  pinMode(28, INPUT);
}

void loop() {
  Serial.print("Voltage: ");
  Serial.print(analogRead(28));
  Serial.println("    Angle: ");
  Serial.println(encoder.angleRead());
  // delay(500);
}
