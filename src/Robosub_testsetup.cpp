/*
 * file: Robosub_testsetup.cpp
 * Platformio code for the Robosub Arduino NANO test setup
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

#include "main.h" // main header file

#define DEBUG // (Serial) DEBUG mode (un)comment to toggle

const int timeBtwnReadings = 500;

// LCD properties
const uint8_t LCD_addr = 0x3f;
const uint8_t LCD_cols = 16; // number of chars on lcd screen
const uint8_t LCD_rows = 2;  // number of lines

LiquidCrystal_I2C lcd(LCD_addr, LCD_cols, LCD_rows); // set the LCD address to 0x27 for a LCD_chars chars and LCD_lines line display

VernierLib Vernier; // create an instance of the VernierLib library

// PIN definitions
uint8_t ESC_PIN = 3;                // Define ESC control pin
uint8_t BTNS[NUM_BTNS] = {4, 7, 8}; // Define ESC control pin D4 D7 D8

// Create a Servo object
Servo esc;

void setup()
{
  Serial.begin(9600);           // initialize serial communication at 9600 bits per second:
  pinMode(LED_BUILTIN, OUTPUT); // specifies that LED_BUILTIN will be used for output
  pinMode(ESC_PIN, OUTPUT);

  lcd.init(); // initialize the lcd  screen
  lcd.backlight();
  lcd.print("Starting..");

  Vernier.autoID(); // this is the routine to do the autoID Serial.println("Vernier Format 2");

#ifdef DEBUG
  Serial.println(Vernier.sensorName());
  Serial.print(" ");
  Serial.println("Readings taken using Ardunio");
  Serial.println("Data Set");
  Serial.print("Time"); // long name
  Serial.print("\t");   // tab character
  Serial.println(Vernier.sensorName());
  Serial.print("t");  // short name
  Serial.print("\t"); // tab character
  Serial.println(Vernier.shortName());
  Serial.print("seconds"); // units
  Serial.print("\t");      // tab character
  Serial.println(Vernier.sensorUnits());
#endif

  esc.attach(ESC_PIN); // Attach the ESC to the specified pin
  armESC();            // Arm the ESC
}

void loop()
{
  readVernier();

  delay(timeBtwnReadings); // stabilize time between readings (FUTURE maybe timer?)
}

/*
  Function:
  Parameters:
 */
void armESC()
{
  esc.writeMicroseconds(1000); // Send a signal to the ESC to arm it
  delay(1000);
}

/*
  Function:
  Parameters:
 */
int readVernier()
{
  float sensorReading = Vernier.readSensor();
  return sensorReading;
}

/*
  Function: calcPower
  Parameters:
 */
float calcPower()
{
  esc.writeMicroseconds(1000); // Send a signal to the ESC to arm it
  delay(1000);
}

/*
Deze functie laat de motor door 9 standen lopen, van 1550 tot 2000. duurt intotaal 90 seconden
*/
void motor_test_ladder(void)
{
  uint8_t i;
  uint8_t thrust = 50;
  for (i = 0; i <= STEPS; i++)
  {
    esc.writeMicroseconds(MINIMUM_THRUST + thrust);
    delay(10000);
    thrust = thrust + THRUST_LADDER;
  }
  esc.writeMicroseconds(MINIMUM_THRUST);
}

/*
Deze functie laat de motor continue harder draaien, duurt intotaal 10 sec.
*/

void motor_test_continuous(void)
{
  uint8_t i;
  uint8_t thrust;
  for (i = 0; i = CYCLES; i++)
  {
    thrust = thrust + i;
    esc.writeMicroseconds(MINIMUM_THRUST + thrust);
    delay(20);
  }
  esc.writeMicroseconds(MINIMUM_THRUST);
}