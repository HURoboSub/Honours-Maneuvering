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
 * Version: 1.6.1
 *
 * CHANGELOG:
 *
 *
 */

#include "main.h" // main header file

#define DEBUG // (Serial) DEBUG mode (un)comment to toggle
<<<<<<< HEAD
#define CAL // Whether to calibrate shunt at the beginning 
#define LCD 0
=======
// #define CAL // Whether to calibrate shunt at the beginning 
>>>>>>> origin/main
// #define USE_VERNIERLIB

enum testPrograms testProgram = B; // default to test program A

/* PIN DEFINTIONS */
uint8_t VOLT_PIN = A0; // Define Voltage control
uint8_t AMP_PIN = A1; // Define Amperage control

uint8_t ESC_PIN = 3; // Define ESC control pin

const uint8_t BUTTON_PINS[NUM_BUTTONS] = {6, 5, 4}; // Define ESC control pin D6 D5 D4
Bounce *buttons = new Bounce[NUM_BUTTONS];          // Initiate 3 Bounce objects
bool buttonStates[NUM_BUTTONS] = {false};           // bool array storing the buttonStates
bool *pButtonStates = &buttonStates[0];             // define pointer, pointing to zeroth element of buttonStates array

/* ADC Calibration values */
float ADC_V_Step = MAX_VOLT / MAX_ADC;
float ADC_A_Step = MAX_AMP / MAX_ADC;

LiquidCrystal_I2C lcd(LCD_addr, LCD_COLS, LCD_ROWS); // set the LCD address to LCD_addr for a LCD_chars by LCD_lines display

char const *rowOneLCD = "               "; // const pointer to a char array containing linezero of LCD
char const *rowTwoLCD = "               "; // textrow one of LCD

/* Motor configuration */
Servo esc; // Create a Servo object

// Create the 3 timers for each motortestprogram
TimerEvent timer_motor_test_a;
TimerEvent timer_motor_test_b;
TimerEvent timer_motor_test_c;

uint8_t timer_expired = 0;
bool continuous_motor_test = true;

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
  pinMode(ESC_PIN, OUTPUT);

  pinMode(VOLT_PIN, INPUT);
  pinMode(AMP_PIN, INPUT);

  pinMode(BUTTON_PINS[0], INPUT_PULLUP);
  pinMode(BUTTON_PINS[1], INPUT_PULLUP);
  pinMode(BUTTON_PINS[2], INPUT_PULLUP);

  // After setting up the button, setup debouncer
  for (int i = 0; i < NUM_BUTTONS; i++)
  {
    buttons[i].attach(BUTTON_PINS[i], INPUT_PULLUP); // setup the bounce instance for the current button
    buttons[i].interval(25);                         // debounce interval in ms
  }

  // measuremunt datastructure
  output2Serial(pData); // output header row to serial

  // https://github.com/YukiSakuma/arduino/blob/a0d36da69587d03019de49ea383efab30b5f0fac/VernierAnalogAutoID/VernierAnalogAutoID.ino#L74C1-L74C102

  
  #ifdef CAL
  Calibrate();

  lcd.setCursor(0, 1);

  lcd.print("Press Green");
  #ifdef DEBUG 
    Serial.println("Press Green");
  #endif

  do
  {
    handleButtons(pButtonStates);
  } while (buttonStates[1] == false); // wacht totdat de meest midelste knop is ingedrukt

  lcd.clear(); // leeghalen lcd scherm
  lcd.home();
  #endif 
<<<<<<< HEAD
  
  // motor
=======

  // motor
  esc.attach(ESC_PIN); // Attach the ESC to the specified pin
>>>>>>> origin/main
  initMotor();         // Initialize the ESC
}

void loop()
{ 
  lastReadTime = millis();
  handleButtons(pButtonStates);

  // pData->force = readVernier();
  // calcPower(pData);
  motorTest(testProgram);
  // output2Serial(pData);
}

/*
  Function: Calibrate the shunt for Voltage and Current
 */
void Calibrate(void)
{
  uint16_t ADCval;
  char strBuf[8]; // convert calculated ADC Step (float) to char-array for printing

  currentState = systemState::Calibrating; 

  /* Current Calibration */
  #ifdef DEBUG 
  userInterface(currentState);
  Serial.println((String)CAL_AMP + "A aansluiten");
  #endif

  lcd.clear();
  lcd.home(); // LCD cursor to 0,0
  lcd.print((String)CAL_AMP + "A aansluiten");  // Show instruction on 1 LCD-row

  do
  {
    handleButtons(pButtonStates);
  } while (buttonStates[0] == false); // Wait until most left button has been pressed

  for (uint8_t i = 0; i < NUM_ADC_READINGS; i++)
    ADCval += analogRead(AMP_PIN); // Read AMP_PIN NUM_ADC_READINGS times and sum it

  ADCval /= NUM_ADC_READINGS; // Calculate average value (total sum / number of readings)

  ADC_A_Step = CAL_AMP / ADCval; // Calculate amount of current per ADC-value step

  dtostrf(ADC_A_Step, 2, 5, strBuf); // Convert float to char-array 
  
  #ifdef DEBUG 
  Serial.println((String)"ADC_A_Step: " + strBuf + " A/Step" );
  #endif
  
  lcd.clear(); // Show calculated current step on LCD
  lcd.home(); // LCD cursor to 0,0
  lcd.print(strBuf);
  lcd.print(" A/Step");

  /* Voltage Calibration */
  #ifdef DEBUG 
  Serial.println((String) + CAL_VOLT + "V aansluiten");
  #endif

  lcd.setCursor(0, 1); // LCD cursor to row 2
  lcd.print((String)CAL_VOLT + "V aansluiten"); // Show instruction on 2nd LCD-row

  do
  {
    handleButtons(pButtonStates);
  } while (buttonStates[0] == false); // Wait until most left button has been pressed

  for (uint8_t i = 0; i < NUM_ADC_READINGS; i++)
    ADCval += analogRead(VOLT_PIN); // Read VOLT_PIN NUM_ADC_READINGS times and sum it

  ADCval /= NUM_ADC_READINGS; // Calculate average value (total sum / number of readings)

  ADC_V_Step = CAL_VOLT / ADCval; // Calculate amount of current per ADC-value step

  dtostrf(ADC_V_Step, 2, 5, strBuf); // Convert float to char-array 
  
  #ifdef DEBUG
  Serial.println((String)"ADC_V_Step: " + strBuf + " V/Step");
  #endif

  lcd.clear(); // Show calculated voltage step on LCD
  lcd.home();
  lcd.print(strBuf);
  lcd.print(" V/Step");
}

/*
  Function:
  Parameters:
 */
void initMotor()
{
<<<<<<< HEAD
  esc.attach(ESC_PIN); // Attach the ESC to the specified pin
  delay(20);
  
  esc.writeMicroseconds(MTR_NEUTRAL); // Send a signal to the ESC to arm it
  delay(MTR_STARTUP_DELAY_MS);
=======
  esc.writeMicroseconds(MINIMUM_THRUST); // Send a signal to the ESC to arm it
  delay(3000);
>>>>>>> origin/main
}

/*
  Function: handle button presses
  Parameters: pS, pointer to i'th index of buttonstatearray
 */
void handleButtons(bool *pState) {

  // for the NUM_BUTTONS increase i and state pointer
  for (int i = 0; i < NUM_BUTTONS; pState++, i++)
  {
    // Update the Bounce instance :
    buttons[i].update();
    
    *pState = buttons[i].fell(); // change right value of this button state

    #ifdef DEBUG 
    if (buttons[i].fell())
    {
        Serial.println((String)"button: " + i + " pressed\t state: " + *pState);
    }
    #endif
  }
}

/*
  Function: Reads varnier sensor and returns value
  Parameters:
 */
// int readVernier()
// {
//   currentState = systemState::Reading; // put system to Reading state

//   float sensorReading = Vernier.readSensor();

//   return sensorReading;
// }

/*
  Function: calcPower
  Calculate power based on voltage and current measurements
  Parameters: PMEASUREMENT p, pointer to measurement data structure
 */
float calcPower(PMEASUREMENT p)
{
  float power = 0; // Initialize power variable
  int ampVal = 0;  // Initialize analog value for current
  int voltVal = 0; // Initialize analog value for voltage

  currentState = systemState::Reading; // put system to Reading state
  userInterface(currentState);

  // Read analog values from pins
  voltVal = analogRead(VOLT_PIN); // Read voltage value
  ampVal = analogRead(AMP_PIN);   // Read current value

  // Convert analog values to actual voltage and current
  p->voltage = (voltVal * ADC_V_Step); // Calculate voltage in volts
  p->current = (ampVal * ADC_A_Step);  // Calculate current in amperes

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
    currentState = systemState::Output; // put system to Output state

    Serial.println("time (ms), force (N), voltage (V), current (mA), power (W)"); // print header row
  }
  else // print data
  {
    currentState = systemState::Output; // put system to Output state
    userInterface(currentState);

    Serial.print(millis() - lastReadTime);
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

  int i;
  uint8_t thrust = 50;

  currentState = systemState::Testing; // put system to Testing 
  #ifdef DEBUG
  Serial.println("Testing motor");
  #endif

  #ifdef DEBUG
    Serial.println((String) "Testing motorprogram:" + (int)prog);
  #endif
  
  switch (prog)
  {
  case A:
    /* Testprogramma A continuous */
    // clockwise
    for (i = MTR_NEUTRAL; i > MTR_MIN_CLOCKWISE ; i-= MRT_INCREMENT)
    {
      esc.writeMicroseconds(i);  // Set the motor to 0 RPM

      #ifdef DEBUG
      Serial.println((String)"thrust:"+ i);
      #endif
      delay(20);
    }

    #ifdef DEBUG
    Serial.println("Reached full clockwise speed:");
    #endif

    // From full speed clockwise to full speed anticlock wise
    for (i = MTR_MIN_CLOCKWISE; i < MTR_MAX_ANTICLOCKWISE ; i+= MRT_INCREMENT)
    {
      esc.writeMicroseconds(i);  // Set the motor to 0 RPM

      #ifdef DEBUG
      Serial.println((String)"thrust:"+ i);
      #endif
      delay(20);
    }

    #ifdef DEBUG
    Serial.println("Reached full anticlockwise speed:");
    #endif

    // TIMER APPROACH
    // timer_motor_test_a.set(DUR_PROG_A, prog_a_timer_handler); // Set the timer
    //       Laat de motor continue harder draaien, duurt DUR_PROG_A msecs*/
    // continuous_motor_test = true;  
    // while(continuous_motor_test) // While loop gets played as long as continuous_motor_test is true
    // {
    //     timer_motor_test_a.update();  // Update the timer 
    //     // Put the vernier sensor read func here (can be another timer if needed)
    //     if(timer_expired >= CYCLES )  // Check if the loop has been played 500 times
    //     {
    //       continuous_motor_test = false;  // Set the bool to false to stop the while loop
    //       timer_expired = 0;  // Reset timer_expired
    //       esc.writeMicroseconds(MTR_NEUTRAL);  // Set the motor to 0 RPM
    //     }
    // initMotor(); // first re-intitilias motor?

    break; /* Program A */

  case B:
  /* Testprogramma B LADDER */
  //  timer_motor_test_b.set(DUR_PROG_B, prog_b_timer_handler);  // Set the timer
  //      Deze functie laat de motor door 9 standen lopen, van 1550 tot 2000. duurt intotaal 90 seconden 
  //    */
  //   continuous_motor_test = true;
  //   while(continuous_motor_test)  // While loop gets played as long as continuous_motor_test is true
  //   {
  //       timer_motor_test_b.update();  // Update the timer
  //       // Put the vernier sensor read func here (can be another timer if needed)
  //       #ifdef DEBUG
  //         Serial.println("in testprogramma B");
  //       #endif
  //       if(timer_expired >= STEPS)  //Check if the loop has been played 9 times
  //       {
  //         continuous_motor_test = false;  //Set bool to false to stop loop
  //         timer_expired = 0;  // Reset timer_expired
  //         esc.writeMicroseconds(MTR_NEUTRAL);  // Set the motor to 0 RPM
  //       }
  //   }
  break; /* Program B */

  case C:
    /*Testprogramma C Ramp
    Dit laat de motor direct op fullspeed gaan
    */

    timer_motor_test_c.set(DUR_PROG_C, prog_c_timer_handler); // Set the timer

    timer_motor_test_c.update(); // Update the timer
    // Put the vernier sensor read func here (can be another timer if needed)
    if (timer_expired >= 1) // Check if the loop has been played 9 times
    {
      continuous_motor_test = false;      // Set bool to false to stop loop
      timer_expired = 0;                  // Reset timer_expired
      esc.writeMicroseconds(MTR_NEUTRAL); // Set the motor to 0 RPM
    }
    break; /* Program C */

  default:
    #ifdef DEBUG
      Serial.println("Verkeerd motor test programma doorgegeven");
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
  uint8_t thrust = 50;

  thrust = thrust + 1;                            // Ramp the thrust up with one
  esc.writeMicroseconds(MTR_NEUTRAL + thrust); // Set motor to 1500+ thrust

  timer_expired += 1; // Add one to timer_expired
}

/*
  Function: prog_b_timer_handler

  Parameters: void
*/
void prog_b_timer_handler(void)
{
  uint8_t thrust = 50;
  
    esc.writeMicroseconds(MTR_NEUTRAL + thrust);     // Set motor to 1500 + thrust
    thrust = thrust + THRUST_LADDER;                    // Add 50 to thrust 
    timer_expired += 1;                                 // Add one to timer_expired
}

/*
  Function: prog_c_timer_handler

  Parameters: void
*/
void prog_c_timer_handler(void)
{ 
    esc.writeMicroseconds(MTR_MAX_ANTICLOCKWISE);       // Set motor to 2000
    timer_expired += 1;                                 // Add one to timer_expired
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
  default: // Should never get in default state 
    rowOneLCD = "S: Error       ";
    rowTwoLCD = "No state passed";
    break;
  }

  
  #ifdef DEBUG
  Serial.println(rowOneLCD); // Show state in Serial
  #endif

  // // delete allocated memory for dispText buffer 
  // for (y = 0; y < LCD_ROWS; y++)
  // {
  //   delete[] dispText[y];
  // }
  // delete[] dispText;

}