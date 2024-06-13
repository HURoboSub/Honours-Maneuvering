/*
 * file: Robosub_testsetup.cpp
 * Platformio code for the Robosub Arduino NANO test setup
 *
 * authors:
 *  Jannick Bloemendal
 *  Niels Redegeld
 *  Thijs Vader
 *  Rutger Janssen
 *
 * Hogeschool Utrecht
 * Date: 29-05-2024
 *
 * Version: 2.0.0
 *
 * CHANGELOG:
 * To Log the serialprints to textfile
 *    1. Open new platform io terminal
 *    2. type command 'pio device monitor > output_logs.txt'
 */

#include "main.h" // main header file

/* */
#define DEBUG // (Serial) DEBUG mode (un)comment to toggle
#define DEBUG_VERNIER
#define CAL_VERNIER
// #define DEBUG_MOTOR

#define CAL_SHUNT // Whether to calibrate shunt at the beginning

#define LCD 1 // Toggle LCD 0 to 1

enum testPrograms testProgram = A; // which testprogram to run

/* PIN DEFINTIONS */
uint8_t VOLT_PIN = A0; // Define Voltage control
uint8_t AMP_PIN = A1;  // Define Amperage control

uint8_t ESC_PIN = 3; // Define ESC control pin

uint8_t VERNIER_PIN = A3; // Define Vernier analog read

const uint8_t BUTTON_PINS[NUM_BUTTONS] = {6, 5, 4}; // Define ESC control pin D6 D5 D4
Bounce *buttons = new Bounce[NUM_BUTTONS];          // Initiate 3 Bounce objects
bool buttonStates[NUM_BUTTONS] = {false};           // bool array storing the buttonStates
bool *pButtonStates = &buttonStates[0];             // define pointer, pointing to zeroth element of buttonStates array

/* ADC Calibration values */
float ADC_V_Step = 0.01852;
float ADC_A_Step = 0.01486;

/* Vernier */
float VERNIER_BIAS = 550.0;

LiquidCrystal_I2C lcd(LCD_addr, LCD_COLS, LCD_ROWS); // set the LCD address to LCD_addr for a LCD_chars by LCD_lines display

char const *rowOneLCD = "               "; // const pointer to a char array containing linezero of LCD
char const *rowTwoLCD = "               "; // textrow one of LCD

/* Motor configuration */
Servo esc; // Create a Servo object

// Store the state of motor test program
// program A,       B,       C
uint8_t motorTestState[3] = {NEUTRAL, NEUTRAL, 0};

// Create the 3 timers for each motortestprogram
TimerEvent timer_motor_test_a;
TimerEvent timer_motor_test_b;
TimerEvent timer_motor_test_c;

TimerEvent timer_motor_test_d;
TimerEvent timer_motor_test_e;

uint8_t timer_expired = 0;
bool continuous_motor_test = true;

uint32_t wait = 0;

uint32_t micros_prog_a = MTR_NEUTRAL;
uint32_t micros_prog_b = MTR_NEUTRAL;

/* Measurement data storage*/
MEASUREMENT data;           // measurement data
PMEASUREMENT pData = &data; // point to datastrucutre

/* Timing configuration */
unsigned long lastReadTime = 0ul;

//
void setup()
{
  currentState = systemState::Setup; // put system to Setup state

  Serial.begin(9600); // initialize serial communication at 9600 bits per second:
  Serial.println("Program starting..");

  lcd.init(); // initialize the lcd  screen
  lcd.backlight();
  userInterface(currentState); // diplay setup state on LCD

  pinMode(LED_BUILTIN, OUTPUT); // specifies that LED_BUILTIN will be used for output

  pinMode(VOLT_PIN, INPUT);
  pinMode(AMP_PIN, INPUT);
  pinMode(VERNIER_PIN, INPUT); // Vernier pin as input

  pinMode(BUTTON_PINS[0], INPUT_PULLUP);
  pinMode(BUTTON_PINS[1], INPUT_PULLUP);
  pinMode(BUTTON_PINS[2], INPUT_PULLUP);

  pinMode(ESC_PIN, OUTPUT);

  // After setting up the button, setup debouncer
  for (int i = 0; i < NUM_BUTTONS; i++)
  {
    buttons[i].attach(BUTTON_PINS[i], INPUT_PULLUP); // setup the bounce instance for the current button
    buttons[i].interval(25);                         // debounce interval in ms
  }

#ifdef CAL_VERNIER
  CalibrateVernier(); /* Calibrate Vernier */
#endif                /* CAL_VERNIER */

#ifdef CAL_SHUNT
  CalibrateShunt(); /* Calibrate shunt */
#endif              /* CAL_SHUNT*/

  // mMasuremunt datastructure
  output2Serial(pData); // output header row to serial

  // motor
  initMotor(); // Initialize the ESC
}

void loop()
{
  lastReadTime = millis();
  handleButtons(pButtonStates);

  calcPower(pData);

  motorTest(testProgram);
  // readVernier();
  output2Serial(pData);
}

/*
  Function: Calibrate the shunt for Voltage and Current
 */
void CalibrateShunt(void)
{
  float ADCval = 0.0;
  char strBuf[8]; // convert calculated ADC Step (float) to char-array for printing

  currentState = systemState::Calibrating;

/* Current Calibration */
#ifdef DEBUG
  userInterface(currentState);
  Serial.println((String)CAL_AMP + "A aansluiten (ROOD + en GEEL)");
#endif

  lcd.clear();
  lcd.home();                                  // LCD cursor to 0,0
  lcd.print((String)CAL_AMP + "A aansluiten"); // Show instruction on 1 LCD-row

  do
  {
    handleButtons(pButtonStates);
  } while (buttonStates[0] == false); // Wait until most left button has been pressed

  for (uint8_t i = 0; i < NUM_ADC_READINGS; i++)
    ADCval += (float)analogRead(AMP_PIN); // Read AMP_PIN NUM_ADC_READINGS times and sum it

  ADCval /= NUM_ADC_READINGS; // Calculate average value (total sum / number of readings)

#ifdef DEBUG
  Serial.println((String) "ADCval average: " + ADCval);
#endif

  ADC_A_Step = (float)CAL_AMP / ADCval; // Calculate amount of current per ADC-value step

  dtostrf(ADC_A_Step, 2, 5, strBuf); // Convert float to char-array

#ifdef DEBUG
  Serial.println((String) "ADC_A_Step: " + ADC_A_Step);
#endif

  lcd.clear(); // Show calculated current step on LCD
  lcd.home();  // LCD cursor to 0,0
  lcd.print(strBuf);
  lcd.print(" A/Step");

  ADCval = 0.0; // reset adcval

/* Voltage Calibration */
#ifdef DEBUG
  Serial.println((String) + CAL_VOLT + "V aansluiten");
#endif

  lcd.setCursor(0, 1);                          // LCD cursor to row 2
  lcd.print((String)CAL_VOLT + "V aansluiten"); // Show instruction on 2nd LCD-row

  do
  {
    handleButtons(pButtonStates);
  } while (buttonStates[0] == false); // Wait until most left button has been pressed

  for (uint8_t i = 0; i < NUM_ADC_READINGS; i++)
    ADCval += (float)analogRead(VOLT_PIN); // Read VOLT_PIN NUM_ADC_READINGS times and sum it

  ADCval /= NUM_ADC_READINGS; // Calculate average value (total sum / number of readings)

  ADC_V_Step = CAL_VOLT / ADCval; // Calculate amount of current per ADC-value step

  dtostrf(ADC_V_Step, 2, 5, strBuf); // Convert float to char-array

#ifdef DEBUG
  Serial.print((String) "Volt ADCval avg: " + ADCval);
  Serial.println((String) "ADC_V_Step: " + strBuf);
#endif

#if defined(LCD) && (LCD == 1)
  lcd.clear(); // Show calculated voltage step on LCD
  lcd.home();
  lcd.print(strBuf);
  lcd.print(" V/Step");
#endif

  //   delay(1000);
  // finished wait for press yellow
  lcd.setCursor(0, 1);
  lcd.print("Press green");
#ifdef DEBUG
  Serial.println("Press green");
#endif

  do
  {
    handleButtons(pButtonStates);
  } while (buttonStates[1] == false); // wacht totdat de meest midelste knop is ingedrukt

  lcd.clear(); // leeghalen lcd scherm
  lcd.home();
}

/*
  Function:
  Parameters:
 */
void CalibrateVernier(void)
{
  float readValue = 0.0;

#if defined(LCD) && (LCD == 1)
  lcd.clear();
  lcd.home();                // LCD cursor to 0,0
  lcd.print("CAL vernier!"); // Show instruction on 1 LCD-row
  lcd.setCursor(0, 1);       // LCD cursor to 0,0
  lcd.print("Press yellow");
#endif

  do
  {
    handleButtons(pButtonStates);
  } while (buttonStates[0] == false); // Wait until most left button has been pressed

  // take average of 10 measurements
  for (uint8_t i = 0; i < NUM_ADC_READINGS; i++)
  {
    readValue += analogRead(VERNIER_PIN); // Read VERNIER_PIN NUM_ADC_READINGS times and sum it
    delayMicroseconds(10);
  }

  readValue /= NUM_ADC_READINGS; // Calculate average value (total sum / number of readings)

  VERNIER_BIAS = readValue;
#ifdef DEBUG_VERNIER
  Serial.print("VERNIER_BIAS:\t");
  Serial.println(VERNIER_BIAS);
#endif

#if defined(LCD) && (LCD == 1)
  lcd.clear();
  lcd.home();               // LCD cursor to 0,0
  lcd.print("Calibrated!"); // Show instruction on 1 LCD-row
#endif
}

/*
  Function:
  Parameters:
 */
void initMotor()
{
  esc.attach(ESC_PIN); // Attach the ESC to the specified pin
  delay(20);

  esc.writeMicroseconds(MTR_NEUTRAL); // Send a signal to the ESC to arm it
  delay(MTR_STARTUP_DELAY_MS);
}

/*
  Function: handle button presses
  Parameters: pS, pointer to i'th index of buttonstatearray
 */
void handleButtons(bool *pState)
{

  // for the NUM_BUTTONS increase i and state pointer
  for (int i = 0; i < NUM_BUTTONS; pState++, i++)
  {
    // Update the Bounce instance :
    buttons[i].update();

    *pState = buttons[i].fell(); // change right value of this button state

#ifdef DEBUG
    if (buttons[i].fell())
      Serial.println((String) "button: " + i + " pressed\t state: " + *pState);
#endif
  }
}

/*
  Function: Reads varnier sensor and returns value
  Parameters:
 */
float readVernier()
{
  float readValue = 0.0;
  float voltage = 0.0;
  float force = 0.0;

  currentState = systemState::Reading; // put system to Reading state

  readValue = analogRead(VERNIER_PIN); // single reading

  pData->force_raw = readValue; // set raw value in measurement data

  // #ifdef DEBUG
  // Serial.print("raw Force (ADC):\t");
  // Serial.print(VERNIER_BIAS);
  // #endif
  // for (uint8_t i = 0; i < NUM_ADC_READINGS; i++)
  //   readValue += analogRead(VERNIER_PIN); // Read VERNIER_PIN NUM_ADC_READINGS times and sum it

  // readValue /= NUM_ADC_READINGS; // Calculate average value (total sum / number of readings)

  readValue -= VERNIER_BIAS; // Correct VERNIER_BIAS

  voltage = (readValue / 1023.0) * 5.0; // ADC terug naar spanning
  force = voltage * 23.45;              // multiply by [N/V]
  // -4.67 [0 - 10 N]
  // -23.45 [0 - 50 N]

  // #ifdef DEBUG_VERNIER
  // Serial.print("voltage vernier:\t");
  // Serial.println(voltage);
  // #endif

  pData->force = force; // set force in structure

  return force; // return calculated force
}

/*
  Function: calcPower
  Calculate power based on voltage and current measurements

  Parameters: PMEASUREMENT p, pointer to measurement data structure
 */
float calcPower(PMEASUREMENT p)
{
  float power = 0.0;   // Initialize power variable
  float ampVal = 0.0;  // Initialize analog value for current
  float voltVal = 0.0; // Initialize analog value for voltage

  currentState = systemState::Reading; // put system to Reading state
  userInterface(currentState);

  // Read analog values from pins
  voltVal = (float)analogRead(VOLT_PIN); // Read voltage value
  delay(20);
  ampVal = (float)analogRead(AMP_PIN); // Read current value

  // Convert analog values to actual voltage and current
  p->voltage = (voltVal * ADC_V_Step); // Calculate voltage in volts
  p->current = (ampVal * ADC_A_Step);  // Calculate current in amperes

#ifdef DEBUG // test
// Serial.print("analog read amp:\t");
// Serial.print((ampVal * ADC_A_Step));
#endif
  power = p->voltage * p->current; // Calculate power using the formula: power = voltage * current

  p->power = power; // Store calculated power in the measurement structure

  return power; // Return the calculated power
}

/*
  Function: output2Serial
  Prints measurement data on Serial
  Parameters: PMEASUREMENT p, pointer to measurement data structure
 */
void output2Serial(PMEASUREMENT p)
{
  if (currentState == systemState::Setup) // if system is in setup mode
  {
    currentState = systemState::Output;                                                           // put system to Output state
    Serial.println("time (ms), force_raw (ADC), force (N), voltage (V), current (A), power (W)"); // print header row
  }
  else // print data
  {
    currentState = systemState::Output; // put system to Output state
    userInterface(currentState);

    Serial.print(millis() - lastReadTime);
    Serial.print(",");
    Serial.print(p->force_raw);
    Serial.print(",");
    Serial.print(p->force);
    Serial.print(",");
    Serial.print(p->voltage, 2);
    Serial.print(",");
    Serial.print(p->current, 2);
    Serial.print(",");
    Serial.println(p->power, 2);
  }
}

/*
Deze functie laat de motor door 9 standen lopen, van 1550 tot 2000. duurt intotaal 90 seconden
*/

/*
  Function: motorTest
  Deze functie laat de motor door CYCLES standen lopen. DUR_PROG_A, DUR_PROG_B
  Parameters: enum testPrograms prog, which tesprogram to run
    deps: TimerEvent.h
 */
void motorTest(enum testPrograms prog)
{
  currentState = systemState::Testing; // put system to Testing

#ifdef DEBUG
  Serial.println((String) "Testing motorprogram:" + (int)prog);
#endif

  switch (prog)
  {
  case A:
    initMotor();
    timer_motor_test_a.set(DUR_PROG_A, prog_a_timer_handler); // Set the timer
    //       Laat de motor continue harder draaien, duurt DUR_PROG_A msecs*/
    continuous_motor_test = true;
    while (continuous_motor_test) // While loop gets played as long as continuous_motor_test is true
    {
      timer_motor_test_a.update(); // Update the timer

      // Put the vernier sensor read func here (can be another timer if needed)
      readVernier();        // force [N]
      calcPower(pData);     // motor [A] & [V]
      output2Serial(pData); // write data to Serial

      if (timer_expired >= CYCLES) // Check if the loop has been played 500 times
      {
        prog = B;
        continuous_motor_test = false;      // Set the bool to false to stop the while loop
        timer_expired = 0;                  // Reset timer_expired
        esc.writeMicroseconds(MTR_NEUTRAL); // Set the motor to 0 RPM
      }
    }

    break; /* Program A */

  case B:
/* Testprogramma B LADDER */
#ifdef DEBUG
    Serial.println("in testprogramma B");
#endif
    timer_motor_test_b.set(DUR_PROG_B, prog_b_timer_handler); // Set the timer
                                                              //      Deze functie laat de motor door 9 standen lopen, van 1550 tot 2000. duurt intotaal 90 seconden
                                                              //    */
    continuous_motor_test = true;
    while (continuous_motor_test) // While loop gets played as long as continuous_motor_test is true
    {
      timer_motor_test_b.update(); // Update the timer

      // Put the vernier sensor read func here (can be another timer if needed)
      readVernier();        // force [N]
      calcPower(pData);     // motor [A] & [V]
      output2Serial(pData); // write data to Serial

      if (timer_expired >= STEPS) // Check if the loop has been played 9 times
      {

        prog = B;

        continuous_motor_test = false;      // Set bool to false to stop loop
        timer_expired = 0;                  // Reset timer_expired
        esc.writeMicroseconds(MTR_NEUTRAL); // Set the motor to 0 RPM
      }
    }
    break; /* Program B */

  case C:
    /* Testprogramma C Ramp
    Dit laat de motor direct op fullspeed gaan*/

    timer_motor_test_c.set(DUR_PROG_C, prog_c_timer_handler); // Set the timer
    while (continuous_motor_test)
    {
      timer_motor_test_c.update(); // Update the timer
      // Put the vernier sensor read func here (can be another timer if needed)
      if (timer_expired >= 4)
      {
        continuous_motor_test = false;      // Set bool to false to stop loop
        timer_expired = 0;                  // Reset timer_expired
        esc.writeMicroseconds(MTR_NEUTRAL); // Set the motor to 0 RPM
      }
    }
    break; /* Program C */

  case D:
    timer_motor_test_d.set(DUR_PROG_A, prog_d_timer_handler); // Set the timer
    while (continuous_motor_test)
    {
      timer_motor_test_c.update(); // Update the timer
      // Put the vernier sensor read func here (can be another timer if needed)
      readVernier();        // force [N]
      calcPower(pData);     // motor [A] & [V]
      output2Serial(pData); // write data to Serial
      if (timer_expired >= 1150)
      {
        continuous_motor_test = false;      // Set bool to false to stop loop
        timer_expired = 0;                  // Reset timer_expired
        esc.writeMicroseconds(MTR_NEUTRAL); // Set the motor to 0 RPM
      }
    }
    break; /* Program C */

  case E:
    timer_motor_test_e.set(DUR_PROG_A, prog_e_timer_handler); // Set the timer
    while (continuous_motor_test)
    {
      timer_motor_test_e.update(); // Update the timer
      // Put the vernier sensor read func here (can be another timer if needed)
      readVernier();        // force [N]
      calcPower(pData);     // motor [A] & [V]
      output2Serial(pData); // write data to Serial

      if (timer_expired >= 1150)
      {
        continuous_motor_test = false;      // Set bool to false to stop loop
        timer_expired = 0;                  // Reset timer_expired
        esc.writeMicroseconds(MTR_NEUTRAL); // Set the motor to 0 RPM
      }
    }
    break; /* Program C */

  default:
#ifdef DEBUG
    Serial.println("ERR: Verkeerd motor test programma doorgegeven");
#endif
    break;
  }
}

/*
  Function: prog_a_timer_handler

  Parameters: void
*/

void prog_a_timer_handler(void)
{
#ifdef DEBUG_MOTOR
  Serial.println((String) "wait is at" + wait);
#endif

  switch (motorTestState[A])
  {
  case NEUTRAL: // 0

    if (wait < 5)
      esc.writeMicroseconds(MTR_NEUTRAL);

    wait += 1;

    if (wait >= WAIT_TIME)
    {
      motorTestState[A] = ADDING; // Next phases is increasing speed
      wait = 0;
    }
    break;

  case ADDING: // 1
#ifdef DEBUG_MOTOR
    Serial.println("Reached case Adding");
#endif

    if (micros_prog_a >= MTR_MAX_ANTICLOCKWISE)
    {
      wait += 1;
      if (wait >= WAIT_TIME)
      {
        motorTestState[A] = SUBTRACTING;
        wait = 0;
      }
    }
    else
    {

      esc.writeMicroseconds(micros_prog_a = micros_prog_a + MRT_INCREMENT);
#ifdef DEBUG_MOTOR
      Serial.println((String) "micros_prog_a = " + micros_prog_a);
#endif
    }
    break; // End of ADDING

  case SUBTRACTING: // 2
#ifdef DEBUG_MOTOR
    Serial.println("Reached case Subtracting");
#endif

    if (micros_prog_a == MTR_MIN_CLOCKWISE)
    {
      wait += 1;

      if (wait >= WAIT_TIME)
      {
        motorTestState[A] = ADDING_HALVE;
        wait = 0;
      }
    }
    else
    {
      esc.writeMicroseconds(micros_prog_a = micros_prog_a - MRT_INCREMENT);
    }
    break; // End of subtracting

  case ADDING_HALVE: // 3
#ifdef DEBUG_MOTOR
    Serial.println("Reached case Adding_halve");
#endif
    if (micros_prog_a == MTR_NEUTRAL)
    {
      wait += 1;
      if (wait >= WAIT_TIME)
      {
        motorTestState[A] = NEUTRAL;
        wait = 0;
      }
    }
    else
      esc.writeMicroseconds(micros_prog_a = micros_prog_a + MRT_INCREMENT);
    break; // End of ADDING_HALVE

  default:
    esc.writeMicroseconds(MTR_NEUTRAL);
    break;
  }
  timer_expired = timer_expired + 1;
}

/*
  Function: prog_b_timer_handler

  Parameters: void
*/
void prog_b_timer_handler(void)
{
  switch (motorTestState[B])
  {
  case NEUTRAL:
    esc.writeMicroseconds(MTR_NEUTRAL);
    motorTestState[B] = UPPER;

#ifdef DEBUG_MOTOR
    Serial.println((String) "micros_prog_b = " + micros_prog_b);
#endif
    break; /* end of case NEUTRAL */

  case UPPER:
    esc.writeMicroseconds(micros_prog_b);
    micros_prog_b = micros_prog_b + 50;
#ifdef DEBUG_MOTOR
    Serial.println((String) "micros_prog_b = " + micros_prog_b);
#endif
    if (micros_prog_b >= MTR_MAX_ANTICLOCKWISE)
      motorTestState[B] = LOWER; // next state of prog B
    break;                       /* end of case UPPER */

  case LOWER:
    esc.writeMicroseconds(micros_prog_b);
    micros_prog_b = micros_prog_b - 50;
#ifdef DEBUG_MOTOR
    Serial.println((String) "micros_prog_b = " + micros_prog_b);
#endif

    if (micros_prog_b <= MTR_MIN_CLOCKWISE)
      motorTestState[B] = FINISHED; // next state of prog B
    break;                          /* end of case LOWER */

  case FINISHED:
    esc.writeMicroseconds(MTR_NEUTRAL);
    break; /* end of case FINISHED */

  default:
    esc.writeMicroseconds(MTR_NEUTRAL);
    break;
  }
  timer_expired += 1; // Add one to timer_expired
}

/*
  Function: prog_c_timer_handler

  Parameters: void
*/
void prog_c_timer_handler(void)

{
  uint32_t i = 0;
#ifdef DEBUG_MOTOR
  Serial.println((String) "State = " + motorTestState[C]);
#endif
  switch (motorTestState[C])
  {
  case 0:
    esc.writeMicroseconds(MTR_NEUTRAL);
    motorTestState[C] = 1; // next state of prog C
    break;
  case 1:
    for (i = MTR_NEUTRAL; i <= MTR_MAX_ANTICLOCKWISE; i++)
    {
      esc.writeMicroseconds(i);
      delay(5);
    }
    motorTestState[C] = 2; // next state of prog C
    break;
  case Third:
    for (i = MTR_MAX_ANTICLOCKWISE; i <= MTR_MIN_CLOCKWISE; i--)
    {
      esc.writeMicroseconds(i);
      delay(5);
    }
    motorTestState[C] = 3; // next state of prog C
    break;
  case 3:
    for (i = MTR_MIN_CLOCKWISE; i <= MTR_NEUTRAL; i++)
    {
      esc.writeMicroseconds(i);
      delay(5);
    }
  default:
    esc.writeMicroseconds(MTR_NEUTRAL);
    break;
  }

  timer_expired += 1; // Add one to timer_expired
}

void prog_d_timer_handler(void)
{
#ifdef DEBUG_MOTOR
  Serial.println((String) "wait is at" + wait);
#endif

  switch (motorTestState[A])
  {
  case NEUTRAL: // 0

    if (wait < 5)
      esc.writeMicroseconds(MTR_NEUTRAL);

    wait += 1;

    if (wait >= WAIT_TIME)
    {
      motorTestState[A] = ADDING; // Next phases is increasing speed
      wait = 0;
    }
    break;

  case ADDING: // 1
#ifdef DEBUG_MOTOR
    Serial.println("Reached case Adding");
#endif

    if (micros_prog_a >= MTR_MAX_ANTICLOCKWISE)
    {
      wait += 1;
      if (wait >= WAIT_TIME)
      {
        motorTestState[A] = SUBTRACTING;
        wait = 0;
      }
    }
    else
    {

      esc.writeMicroseconds(micros_prog_a = micros_prog_a + MRT_INCREMENT);
#ifdef DEBUG_MOTOR
      Serial.println((String) "micros_prog_a = " + micros_prog_a);
#endif
    }
    break; // End of ADDING

  case SUBTRACTING: // 2
#ifdef DEBUG_MOTOR
    Serial.println("Reached case Subtracting");
#endif

    if (micros_prog_a == MTR_NEUTRAL)
    {
      wait += 1;

      if (wait >= WAIT_TIME)
      {
        motorTestState[A] = NEUTRAL;
        wait = 0;
      }
    }
    else
    {
      esc.writeMicroseconds(micros_prog_a = micros_prog_a - MRT_INCREMENT);
    }
    break; // End of subtracting
  }
}

void prog_e_timer_handler(void)
{
#ifdef DEBUG_MOTOR
  Serial.println((String) "wait is at" + wait);
#endif

  switch (motorTestState[A])
  {
  case NEUTRAL: // 0

    if (wait < 5)
      esc.writeMicroseconds(MTR_NEUTRAL);

    wait += 1;

    if (wait >= WAIT_TIME)
    {
      motorTestState[A] = SUBTRACTING; // Next phases is increasing speed
      wait = 0;
    }
    break;

  case SUBTRACTING: // 1
#ifdef DEBUG_MOTOR
    Serial.println("Reached case Adding");
#endif

    if (micros_prog_a >= MTR_MAX_ANTICLOCKWISE)
    {
      wait += 1;
      if (wait >= WAIT_TIME)
      {
        motorTestState[A] = ADDING;
        wait = 0;
      }
    }
    else
    {

      esc.writeMicroseconds(micros_prog_a = micros_prog_a - MRT_INCREMENT);
#ifdef DEBUG_MOTOR
      Serial.println((String) "micros_prog_a = " + micros_prog_a);
#endif
    }
    break; // End of ADDING

  case ADDING: // 2
#ifdef DEBUG_MOTOR
    Serial.println("Reached case Subtracting");
#endif

    if (micros_prog_a == NEUTRAL)
    {
      wait += 1;

      if (wait >= WAIT_TIME)
      {
        motorTestState[A] = NEUTRAL;
        wait = 0;
      }
    }
    else
    {
      esc.writeMicroseconds(micros_prog_a = micros_prog_a + MRT_INCREMENT);
    }
    break; // End of subtracting
  }
}

/*
  Function: LCD_show
    Handles the lcd display (clear, cursor and print) based on LCD_ROWS and LCD_COLS
  Parameter: pointer to a string in a 2d
 */

// void LCD_show(char **str)
// {
//   unsigned char x, y; // x and y loop index

//   lcd.clear(); // clear the display

//   // for each row copy textlines to dispText char arrays
//   for (y = 0; y < LCD_ROWS; y++)
//   {
//     for (x = 0; x < LCD_COLS; x++)
//     {
//       str[0][x] = rowOneLCD[x];
//       str[1][x] = rowTwoLCD[x];
//     }
//   }
//   // Display the contents of the display buffer on the LCD screen
//   for (y = 0; y < LCD_ROWS; y++, str++)
//   {
//     lcd.setCursor(0, y);
//     lcd.print(*str); // print the string buffer
//   }
// }

/*
  Function: userInterface
    Handles different system states on the LCD screen
    Parameters: class enumator with the current State
 */
void userInterface(systemState cState)
{
  // unsigned char y; // y loop index

  // // Memory allocation for the display text
  // char **dispText = new char *[LCD_ROWS];

  // for (y = 0; y < LCD_ROWS; y++)
  // {
  //   dispText[y] = new char[LCD_COLS];
  // }

  switch (cState)
  {
  case systemState::Setup:
    // Setup state here
    rowOneLCD = "S: Setup       ";
    break;
  case systemState::Calibrating:
    // Calibrating state here
    rowOneLCD = "S: Calibrating ";
    break;
  case systemState::Reading:
    // S2 here
    rowOneLCD = "S: Reading     ";
    break;
  case systemState::Testing:
    // S3 Testing here
    rowOneLCD = "S: Testing     ";
    break;
  case systemState::Output:
    // S4 Output here
    rowOneLCD = "S: Output      ";
    break;
  default: // Should never get in default stateú
    rowOneLCD = "S: Error       ";
    rowTwoLCD = "No state passed";
    break;
  }

  // // delete allocated memory for dispText buffer
  // for (y = 0; y < LCD_ROWS; y++)
  // {
  //   delete[] dispText[y];
  // }
  // delete[] dispText;
}
