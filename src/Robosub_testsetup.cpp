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
 * Date: 24-09-2024
 *
 * Version: 2.3.0
 * 
 * To Log the serialprints to textfile
 *    1. Open new platform io terminal
 *    2. type command 'pio device monitor > output_logs.txt'
 */

#include "main.h" // Main header 

#define DEBUG // (un)comment to toggle (Serial) DEBUG mode 
// #define DEBUG_VERNIER
// #define DEBUG_MOTOR

#define CAL_VERNIER // Whether to calibrate Vernier at startup
#define CAL_SHUNT // Whether to calibrate shunt at startup

#define LCD 1 // Toggle LCD 0 to 1

unsigned long lastReadTime = 0ul; // Track of time

systemState currentState; // class storing the current system state
enum testPrograms testProgram; // which testprogram to run

/* Measurement data storage*/
MEASUREMENT data;           // measurement data
PMEASUREMENT pData = &data; // point to datastructure

/* Button pin array */
const uint8_t BUTTON_ARRAY[NUM_BUTTONS] = 
  { BTN_YELLOW_PIN,
    BTN_GREEN_PIN,
    BTN_BLUE_PIN }; 

Bounce *buttons = new Bounce[NUM_BUTTONS];          // Initiate 3 Bounce objects
bool buttonStates[NUM_BUTTONS] = {false};           // bool array storing the buttonStates

/* ADC Calibration values */
float ADC_V_Step = 0.01852;
float ADC_A_Step = 0.01486;

/* LCD */
LiquidCrystal_I2C lcd(LCD_addr, LCD_COLS, LCD_ROWS); // set the LCD address to LCD_addr for a LCD_chars by LCD_lines display

char const *rowOneLCD = "               "; // const pointer to a char array containing linezero of LCD
char const *rowTwoLCD = "               "; // textrow one of LCD

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

  pinMode(BUTTON_ARRAY[0], INPUT_PULLUP);
  pinMode(BUTTON_ARRAY[1], INPUT_PULLUP);
  pinMode(BUTTON_ARRAY[2], INPUT_PULLUP);

  pinMode(ESC_PIN, OUTPUT);

  // After setting up the button, setup debouncer
  for (int i = 0; i < NUM_BUTTONS; i++)
  {
    buttons[i].attach(BUTTON_ARRAY[i], INPUT_PULLUP); // setup the bounce instance for the current button
    buttons[i].interval(25);                         // debounce interval in ms
  }

#ifdef CAL_VERNIER
  CalibrateVernier(); /* Calibrate Vernier */
#endif                /* CAL_VERNIER */

#ifdef CAL_SHUNT
  CalibrateShunt(); /* Calibrate shunt */
#endif              /* CAL_SHUNT*/

  testProgram = selectProgram(); // Ask to select program
  output2Serial(pData); // output header row to serial
  
  initMotor(); // Initialize the ESC
}

void loop()
{
  lastReadTime = millis();
  bool *pButtonStates = &buttonStates[0]; // define pointer, pointing to zeroth element of buttonStates array

  handleButtons(pButtonStates);

  calcPower(pData);
  motorTest(testProgram);
}
/*
  Function: handle button presses
  Parameters: pState, pointer to i'th index of buttonstatearray
 */
void handleButtons(bool *pState)
{
  // for the NUM_BUTTONS increase i and state pointer
  for (int i = 0; i < NUM_BUTTONS; pState++, i++)
  {
    buttons[i].update(); // Update the Bounce instance

    *pState = buttons[i].fell(); // change right value of this button state

#ifdef DEBUG
    if (buttons[i].fell())
      Serial.println((String) "button: " + i + " pressed\t state: " + *pState);
#endif
  }
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

  waitforButton(YELLOW); // Wait until yellow button has been pressed

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

  waitforButton(YELLOW); // Wait until yellow button has been pressed

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

  // finished wait for press yellow
  lcd.setCursor(0, 1);
  lcd.print("Press green");
#ifdef DEBUG
  Serial.println("Press green");
#endif

  waitforButton(GREEN); // Wait until green button has been pressed

  lcd.clear(); // leeghalen lcd scherm
  lcd.home();
}


/*
  Function: selectProgram(
  lets the user select a motor program via a button press
  Parameters: void
 */
testPrograms selectProgram(void)
{
  testPrograms selectedProg = A;
  bool progSelected = false;
  bool *pButtonStates = &buttonStates[0]; // define pointer, pointing to zeroth element of buttonStates array

  #ifdef DEBUG
    Serial.println("SelectProgram()");
  #endif

  while (!progSelected)
  {
    handleButtons(pButtonStates);

    if (buttonStates[YELLOW]) // Yellow is pressed selectedProg down
    {
      selectedProg = (testPrograms)((selectedProg +1) % NUM_PROGRAMS);
    }
    if (buttonStates[GREEN])
    {
      selectedProg = (testPrograms)((selectedProg - 1 + NUM_PROGRAMS) % NUM_PROGRAMS); // Green is pressed up
    }

    #if defined(LCD) && (LCD == 1)
      lcd.home();
      if (selectedProg == A)
      {
        lcd.print("Prog A?");
      }
      else if (selectedProg == B)
      {
        lcd.print("Prog B?");
      }
      else 
      {
        lcd.print("Prog C?");
      }
      
      lcd.setCursor(0, 1);
      lcd.print("Blue to confirm");
    #endif

    if (buttonStates[BLUE]) // Confirm button pressed
    {
      progSelected = true;

      #if defined(LCD) && (LCD == 1)
      lcd.clear();
      lcd.home();
      lcd.print("Prog selected:");

      lcd.setCursor(0, 1);
      #endif

      switch (selectedProg)
      {
      case A:
        lcd.print("A");
        #ifdef DEBUG
          Serial.println("Prog A selected");
        #endif // DEBUG
        break;
      case B:
        lcd.print("B");
        #ifdef DEBUG
          Serial.println("Prog B selected");
        #endif // DEBUG
        break;
      case C:
        lcd.print("C");
        #ifdef DEBUG
          Serial.println("Prog C selected");
        #endif // DEBUG
        break;
      default:
        break;
      }
    
    }
  }

  return selectedProg;
}

/*
  Function: wait for single button to be pressed
  Parameters: 
    enum buttonIndices btn_i: index of the button (YELLOW, GREEN, BLUE)
 */
void waitforButton(enum buttonIndices btn_i)
{
  #ifdef DEBUG
  Serial.print("Waiting for buttonpress of: ");
  Serial.println(btn_i);
  #endif
  
  if (btn_i >= 0 && btn_i <= NUM_BUTTONS) // if btn_i is a valid value
  {
    do 
    {
      buttons[btn_i].update(); // Update the Bounce instance

      buttonStates[btn_i] = buttons[btn_i].fell(); // change right value of this button state

      if (buttons[btn_i].fell())
      {
        #ifdef DEBUG
        Serial.print("button: ");
        Serial.print(btn_i);
        Serial.println((String) " pressed\t state: " + buttonStates[btn_i]);
        #endif
      }
      
    } while (buttonStates[btn_i] == false); // wait until button with btn_i is pressed
  }
  else
  {
    #ifdef DEBUG
    Serial.println((String) "Error btn_i not in NUM_BUTTONS");
    #endif
  }
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
  if (currentState == systemState::Setup || currentState == systemState::Calibrating) // if system is in setup or Calibrating mode
  {
    currentState = systemState::Output;                                                           // put system to Output state
    Serial.println("Time (ms), Force_raw (ADC), Force (N), Voltage (V), Current (A), Power (W)"); // print header row
  }
  else // print data
  {
    currentState = systemState::Output; // put system to Output state
    userInterface(currentState);

    Serial.print(millis() - lastReadTime); // time in ms
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
