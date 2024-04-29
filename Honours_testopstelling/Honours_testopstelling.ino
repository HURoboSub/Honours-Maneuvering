/*H**********************************************************************
* FILENAME :        Honours_testopstelling.ino
*
* DESCRIPTION :
* VERSION     :   1.0
*
* CHANGELOG :
*   29/04/2024 - toevoegen header en hernoemen bestand
*
*H*/
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
  esc.writeMicroseconds(2000);
  delay(5000);
  esc.writeMicroseconds(1900);
  delay(5000);
  esc.writeMicroseconds(1800);
  delay(5000);
  esc.writeMicroseconds(1700);
  delay(5000);
  esc.writeMicroseconds(1600);
  delay(5000);
  esc.writeMicroseconds(1500);
  delay(5000);

}

void armESC() {
  // Send a signal to the ESC to arm it
  esc.writeMicroseconds(1000);
  delay(1000);
}