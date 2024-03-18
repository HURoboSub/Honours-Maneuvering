#include <Servo.h>

// Define ESC control pin
int escPin = 9;

// Create a Servo object
Servo esc;

void setup() {
  // Attach the ESC to the specified pin
  esc.attach(escPin);

  // Arm the ESC
  armESC();
}

void loop() {
  // Sweep the motor speed from minimum to maximum
  for (int speed = 1000; speed <= 2000; speed += 10) {
    esc.writeMicroseconds(speed);
    delay(50);
  }

  // Sweep the motor speed back from maximum to minimum
  for (int speed = 2000; speed >= 1000; speed -= 10) {
    esc.writeMicroseconds(speed);
    delay(50);
  }
}

void armESC() {
  // Send a signal to the ESC to arm it
  esc.writeMicroseconds(1000);
  delay(1000);
}