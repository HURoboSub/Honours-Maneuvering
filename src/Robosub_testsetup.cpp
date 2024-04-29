/*
 * file: Robosub_testsetup.cpp
 * Platformio Arduino code for the Robosub UNO test setup 
 *
 * authors: 
 *  Jannick Bloemendal
 *  Niels Redegeld
 *  Thijs Vader
 *  Rutger Jansen
 * 
 * Hogeschool Utrecht
 * Date: 29-04-2024
 * 
 * Version: 1.1.0
 * 
 * CHANGELOG:
 * 
 * 
 */

#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h> // LCD SCREEN
#include <Servo.h>
// button debounce lib
// 

#define DEBUG // (Serial) debug mode (un)comment to toggle

// pin definitions
int escPin = 9; // Define ESC control pin

int VernierPin = A0; //Vernier force interface

// LCD i2c
// x times a button

// Create a Servo object
Servo esc;
Servo *pEsc = &esc; // !! DIT nog slim aanpassen met pointers

void armESC();
int readVernier(int pin);
void sweepMotor(Servo *pESC);

void setup() {
  Serial.begin(9600);  // initialize serial communication at 9600 bits per second:
  
  esc.attach(escPin); // Attach the ESC to the specified pin
 
  armESC();  // Arm the ESC
}

void loop() {
  readVernier(VernierPin);
  sweepMotor(pEsc);
}

void armESC() {
  esc.writeMicroseconds(1000);  // Send a signal to the ESC to arm it
  delay(1000);
}

/*
  Function:
  Parameters: the AnalogPin to where the Vernier is connected to
 */
int readVernier(int pin) {
  float sensorVoltage; // declares a variable named sensorVoltage
  // read the input on analog pin 0:
  int sensorValue = analogRead(pin);
  sensorVoltage = sensorValue * 5.0 / 1023;// Converts the count to the sensor voltage
  // print out the value you read:
  Serial.println(sensorVoltage);

  delay(500);        // delay in between reads in milliseconds
  return sensorVoltage;
}

/*
  Function: sweepMotor 
  params: pointer to Servo object
*/
void sweepMotor(Servo *pESC){
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