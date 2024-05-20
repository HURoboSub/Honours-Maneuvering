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
 * Date: 14-05-2024
 *
 * Version: 1.5.0
 *
 * CHANGELOG:
 *
 *
 */

#include "main.h" // main header file

// #define DEBUG // (Serial) DEBUG mode (un)comment to toggle
 #define CAL    

 /* ADC Calibration values */
float ADC_V_Step = MAX_VOLT / MAX_ADC;
float ADC_A_Step = MAX_AMP / MAX_ADC;

/* Timing configuration */
const int timeBtwnReadings = 500; // time between Vernierr
unsigned long lastReadTime = 0ul;

LiquidCrystal_I2C lcd(LCD_addr, LCD_COLS, LCD_ROWS); // set the LCD address to LCD_addr for a LCD_chars by LCD_lines display

char const *rowOneLCD = "               ";   // const pointer to a char array containing linezero of LCD
char const *rowTwoLCD = "               "; // textrow one of LCD

/* PIN DEFINTIONS */
uint8_t VOLT_PIN = A0; // Define Voltage control
uint8_t AMP_PIN = A1;  // Define Amperage control

uint8_t ESC_PIN = 3; // Define ESC control pin

const uint8_t BUTTON_PINS[NUM_BUTTONS] = {4, 7, 8}; // Define ESC control pin D4 D7 D8
Bounce *buttons = new Bounce[NUM_BUTTONS];          // Initiate 3 Bounce objects
bool buttonStates[NUM_BUTTONS] = {false};           // bool array storing the buttonStates
bool *pButtonStates = &buttonStates[0];             // define pointer, pointing to zeroth element of buttonStates array

Servo esc; // Create a Servo object

MEASUREMENT data;           // measurement data
PMEASUREMENT pData = &data; // point to datastrucutre

enum testPrograms testProgram = A; // default to test program A

// 
void setup()
{
  currentState = systemState::Setup; // put system to Setup state

  Serial.begin(9600); // initialize serial communication at 9600 bits per second:
  Serial.println("Program starting..");

  lcd.init(); // initialize the lcd  screen
  lcd.backlight();
  userInterface(currentState);// diplay setup state on LCD

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

  // motor
  esc.attach(ESC_PIN); // Attach the ESC to the specified pin
  initMotor();         // Initialize the ESC


  Calibrate();


 // Serial.println("Druk op de knop om te starten");

  lcd.setCursor(0, 1);
  lcd.print("Druk op Groen");
  do {
    handleButtons(pButtonStates);
  } while (buttonStates[1] == false);   // wachten totdat de meest linker button is ingedrukt geweest

  lcd.clear();
  lcd.home();
}

void loop()
{ 
  lastReadTime = millis();
  handleButtons(pButtonStates);

  // pData->force = readVernier();
  calcPower(pData);
  motorTest(testProgram);
  output2Serial(pData);
}

void Calibrate()
{ 

  uint16_t ADCval;
  char* floatString = "                ";

  /* Amps Calibration */
  // lcd 

  //Serial.println((String)"Sluit " + CAL_AMP + "A aan op de testopstelling");
  lcd.clear();
  lcd.home();
  lcd.print((String) CAL_AMP + "A aansluiten");

  do {
    handleButtons(pButtonStates);
  } while (buttonStates[0] == false);   // wachten totdat de meest linker button is ingedrukt geweest

  for(uint8_t i = 0; i < NUM_ADC_READINGS; i++)
      ADCval += analogRead(AMP_PIN);   // 10x meten, gemiddelde pakken
  
  ADCval /= NUM_ADC_READINGS;

  ADC_A_Step = CAL_AMP / ADCval;

  /* Voltage Calibration */
  // lcd 
 // Serial.println((String)"Sluit " + CAL_VOLT + "V aan op de testopstelling");
  lcd.clear();
  lcd.home();
  
  dtostrf(ADC_A_Step, 2, 6, floatString);
  lcd.print(floatString);
  lcd.print(" A/Step");
  lcd.setCursor(0, 1);

  lcd.print((String) CAL_VOLT + "V aansluiten");

  do {
    handleButtons(pButtonStates);
  } while (buttonStates[0] == false);   // wachten totdat de meest linker button is ingedrukt geweest

  for(uint8_t i = 0; i < NUM_ADC_READINGS; i++)
      ADCval += analogRead(VOLT_PIN);   // 10x meten, gemiddelde pakken
  
  ADCval /= NUM_ADC_READINGS;

  ADC_V_Step = CAL_VOLT / ADCval;

  lcd.clear();
  lcd.home();

  dtostrf(ADC_V_Step, 2, 6, floatString);
  lcd.print(floatString);
  lcd.print(" V/Step");
}

/*
  Function:
  Parameters:
 */
void initMotor()
{
  esc.writeMicroseconds(1000); // Send a signal to the ESC to arm it
  delay(1000);
}

/*
  Function: handle button presses
  Parameters: pS, pointer to i'th index of buttonstatearray
 */
void handleButtons(bool *pState) {
  currentState = systemState::Reading;  // put system to Reading state
  userInterface(currentState);

  // for the NUM_BUTTONS increase i and state pointer
  for (int i = 0; i < NUM_BUTTONS; pState++, i++) {
    // Update the Bounce instance :
    buttons[i].update();

    *pState = buttons[i].fell();  // change right value of this button state
    
    #ifdef DEBUG
      Serial.println((String) "i:" + i + "\t state:" + *pState);
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
//   delay(timeBtwnReadings); // stabilize time between readings (!!improve FUTURE maybe timer?)

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
  p->current = (ampVal * ADC_A_Step);    // Calculate current in amperes

  power = p->voltage * p->current; // Calculate power using the formula: power = voltage * current

  p->power = power; // Store calculated power in the measurement structure

  return power; // Return the calculated power
}

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
void motorTest(enum testPrograms prog)
{

  uint8_t i;
  uint8_t thrust = 50;

  currentState = systemState::Testing; // put system to Testing state
  userInterface(currentState);
  
  switch (prog)
  {
  case A:
    /* Testprogramma A continuous
          Laat de motor continue harder draaien, duurt DUR_PROG_A msecs*/
    for (i = 0; i == CYCLES; i++)
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
    for (i = 0; i <= STEPS; i++)
    {
      esc.writeMicroseconds(MINIMUM_THRUST + thrust);
      delay(DUR_PROG_B);
      thrust = thrust + THRUST_LADDER;
    }
    esc.writeMicroseconds(MINIMUM_THRUST);

    break;

  default:
#ifdef DEBUG
    Serial.println("Verkeerd motor test programma doorgegeven");
#endif
    break;
  }
}

/*
  Function: LCD_show
    Handles the lcd display (clear, cursor and print) based on LCD_ROWS and LCD_COLS
  Parameter: pointer to a string in a 2d
 */

void LCD_show(char **str)
{
  unsigned char x,y; // x and y loop index

  lcd.clear(); // clear the display

  // for each row copy textlines to dispText char arrays
  for (y = 0; y < LCD_ROWS; y++)
  {
      for (x = 0; x < LCD_COLS; x++)
      {
          str[0][x] = rowOneLCD[x];
          str[1][x] = rowTwoLCD[x];
      }
  }
  // Display the contents of the display buffer on the LCD screen
  for (y = 0; y < LCD_ROWS; y++, str++)
  {
    lcd.setCursor(0, y);
    lcd.print(*str); // print the string buffer
  }
}

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

  // switch (cState)
  // {
  // case systemState::Setup:
  //   // Setup state here
  //   rowOneLCD = "S: Setup       ";
  //   break;
  // case systemState::Calibrating:
  //   // Calibrating state here
  //   rowOneLCD = "S: Calibrating ";
  //   break;
  // case systemState::Reading:
  //   // S2 here
  //   rowOneLCD = "S: Reading     ";
  //   break;
  // case systemState::Testing:
  //   // S3 Testing here
  //   rowOneLCD = "S: Testing     ";
  //   break;
  // case systemState::Output:
  //   // S4 Output here
  //   rowOneLCD = "S: Output      ";
  //   break;
  // default: // Should never get in default state 
  //   rowOneLCD = "S: Error       ";
  //   rowTwoLCD = "No state passed";
  //   break;
  // }

  // // Show the updated display
  // LCD_show(dispText);

  // // delete allocated memory for dispText buffer 
  // for (y = 0; y < LCD_ROWS; y++)
  // {
  //   delete[] dispText[y];
  // }
  // delete[] dispText;

}