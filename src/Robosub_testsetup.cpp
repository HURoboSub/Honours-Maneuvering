/*
 * file: Robosub_testsetup.cpp
 * VERY BASIC ANALOG READ and print ON pin A3
 *
 * authors:
 *  Jannick Bloemendal
 *  Niels Redegeld
 *  Thijs Vader
 *  Rutger Janssen
 *
 * Hogeschool Utrecht
 * Date: 07-05-2024
 *
 * Version: 1.4.0
 *
 * CHANGELOG:
 *
 *
 */

#include "main.h" // main header file

#define DEBUG // (Serial) DEBUG mode (un)comment to toggle
/*
Read the information that is stored on a Vernier LabQuest sensor
and prints it to the Serial Monitor, and then take readings at 
2 samples/second (0.5 seconds/sample) and print the sensor 
measurements to the Serial Monitor.

 - Connect the Vernier Arduino Interface Shield to a RedBoard, UNO R3 or R4.
 - Connect a Vernier LabQuest sensor to ANALOG 1
 - Upload the code and then view the output in the Serial Monitor

*/


void setup();
void loop();

// Read the sensor information in the 'setup()'
void setup()
{
  Serial.begin(9600);
  while (!Serial);
  delay(4000);   //Need time for the Serial Monitor to become available

}

// Read sensor measurments continuously in the 'loop()'
void loop()
{
  float sensorReading = analogRead(A3);
  #ifdef DEBUG 
    Serial.println(sensorReading);
  #endif
  delay(500);   //half a second delay between readings
}