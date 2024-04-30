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
 * Date: 30-04-2024
 *
 * Version: 1.2.0
 *
 * CHANGELOG:
 *
 *
 */

#include "main.h" // main header file

#define DEBUG // (Serial) DEBUG mode (un)comment to toggle

const int timeBtwnReadings = 500;
unsigned long lastReadTime = 0ul;

// LCD properties
const uint8_t LCD_addr = 0x3f;
const uint8_t LCD_cols = 16; // number of chars on lcd screen
const uint8_t LCD_rows = 2;  // number of lines

LiquidCrystal_I2C lcd(LCD_addr, LCD_cols, LCD_rows); // set the LCD address to 0x27 for a LCD_chars chars and LCD_lines line display

VernierLib Vernier; // create an instance of the VernierLib library

// PIN definitions
uint8_t VOLT_PIN = A0; // Define Voltage control
uint8_t AMP_PIN = A1;  // Define Amperage control

uint8_t ESC_PIN = 3;                // Define ESC control pin
uint8_t BTNS[NUM_BTNS] = {4, 7, 8}; // Define ESC control pin D4 D7 D8

// Create a Servo object
Servo esc;

// measurement data
MEASUREMENT data;
PMEASUREMENT pData;


void setup()
{
  Serial.begin(9600);           // initialize serial communication at 9600 bits per second:
  pinMode(LED_BUILTIN, OUTPUT); // specifies that LED_BUILTIN will be used for output
  pinMode(ESC_PIN, OUTPUT);

  pinMode(VOLT_PIN, INPUT);
  pinMode(AMP_PIN, INPUT);

  pinMode(BTNS[0], INPUT);
  pinMode(BTNS[1], INPUT);
  pinMode(BTNS[2], INPUT);

  pData = &data; // point to datastrucutre
  enum testPrograms program = A; // default to test program A

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
  lastReadTime = millis();

  readVernier();
  calcPower(pData);
  output2Serial(pData);

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
  Calculate power based on voltage and current measurements
  Parameters:
 */
float calcPower(PMEASUREMENT p)
{
  float power = 0; // Initialize power variable
  int ampVal = 0;  // Initialize analog value for current
  int voltVal = 0; // Initialize analog value for voltage

  // Read analog values from pins
  voltVal = analogRead(VOLT_PIN); // Read voltage value
  ampVal = analogRead(AMP_PIN);   // Read current value

  // Convert analog values to actual voltage and current
  p->voltage = (voltVal * VOLTS_ADC_STEP); // Calculate voltage in volts
  p->current = (ampVal * AMS_ADC_Step);    // Calculate current in amperes

  // Calculate power using the formula: power = voltage * current
  power = p->voltage * p->current;

  // Store calculated power in the measurement structure
  p->power = power;

  return power; // Return the calculated power
}

void output2Serial(PMEASUREMENT p)
{
  Serial.print(millis() - lastReadTime);
  Serial.print(",");
  Serial.println(p->voltage, 2);
  Serial.print(",");
  Serial.println(p->current, 2);
  Serial.print(",");
  Serial.println(p->power, 2);
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
Deze functie laat de motor door 9 standen lopen, van 1550 tot 2000. duurt intotaal 90 seconden
*/
void motorTest(enum testPrograms prog)
{
  uint8_t i;
  uint8_t thrust = 50;
  switch (prog)
  {
  case A:

    /* Testprogramma A continuous
           Deze functie laat de motor continue harder draaien, duurt intotaal 10 sec.*/
    for (i = NULL; i = CYCLES; i++)
    {
        thrust = thrust + i;
        esc.writeMicroseconds(MINIMUM_THRUST + thrust);
        delay(DUR_PROG_A);
    }
    esc.writeMicroseconds(MINIMUM_THRUST);

    break;

  case B:
    /* Testprogramma B ladder 
       Deze functie laat de motor door 9 standen lopen, van 1550 tot 2000. duurt intotaal 90 seconden 
     */
    for(i = NULL; i <= STEPS; i++)
    {
        esc.writeMicroseconds(MINIMUM_THRUST + thrust);
        delay(DUR_PROG_B);
        thrust = thrust + THRUST_LADDER;
    }
    esc.writeMicroseconds(MINIMUM_THRUST);

    break;

  default:
    Serial.println("Verkeerd motor test programma doorgegeven");
    break;
  }
}