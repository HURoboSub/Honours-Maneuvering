#include <Servo.h>
//debounce
// liquid crystal
// 

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

/*
  Read Vernier sensor
  
*/
// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {
  float sensorVoltage; // declares a variable named sensorVoltage
  // read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  sensorVoltage = sensorValue * 5.0 / 1023;// Converts the count to the sensor voltage
  // print out the value you read:
  Serial.println(sensorVoltage);
  delay(500);        // delay in between reads in milliseconds
}
