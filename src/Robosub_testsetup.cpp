/*
 * file: Robosub_testsetup.cpp
 * Platformio code for the Robosub Arduino NANO test setup
 * SIMPLIFIED BUTTON TEST setup
 *
 * authors:
 *  Jannick Bloemendal
 *  Niels Redegeld
 *  Thijs Vader
 *  Rutger Janssen
 *
 * Hogeschool Utrecht
 * Date: 30-05-2024
 *
 * Version: 1.6.0
 *
 * CHANGELOG:
 *
 *
 */

#include "main.h" // main header file

#define DEBUG // (Serial) DEBUG mode (un)comment to toggle

/* PIN DEFINTIONS */

const uint8_t BUTTON_PINS[NUM_BUTTONS] = {4, 7, 8}; // Define ESC control pin D4 D7 D8
Bounce *buttons = new Bounce[NUM_BUTTONS];          // Initiate 3 Bounce objects
bool buttonStates[NUM_BUTTONS] = {false};           // bool array storing the buttonStates
bool *pButtonStates = &buttonStates[0];             // define pointer, pointing to zeroth element of buttonStates array

LiquidCrystal_I2C lcd(LCD_addr, LCD_COLS, LCD_ROWS); // set the LCD address to LCD_addr for a LCD_chars by LCD_lines display

char const *rowOneLCD = "               "; // const pointer to a char array containing linezero of LCD
char const *rowTwoLCD = "               "; // textrow one of LCD

Servo esc; // Create a Servo object

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
  
  pinMode(BUTTON_PINS[0], INPUT_PULLUP);
  pinMode(BUTTON_PINS[1], INPUT_PULLUP);
  pinMode(BUTTON_PINS[2], INPUT_PULLUP);

  // After setting up the button, setup debouncer
  for (int i = 0; i < NUM_BUTTONS; i++)
  {
    buttons[i].attach(BUTTON_PINS[i], INPUT_PULLUP); // setup the bounce instance for the current button
    buttons[i].interval(25);                         // debounce interval in ms
  }


}

void loop()
{ 
  lastReadTime = millis();
  handleButtons(pButtonStates);

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
    if (buttons[i].fell()) // If this button fell, print the code
    {
      Serial.println((String)"button[" + i + "] pressed\t state: " + *pState);

      lcd.home();
      lcd.print("button[");
      lcd.print(i);
      lcd.print("]:");
      lcd.setCursor(0, 1);
      lcd.print("state:");
      lcd.print(*pState);
    }
    else if (buttons[i].rose()) // button released, clear the display
    {
      Serial.println((String)"button[" + i + "] released\t state: " + *pState);
      lcd.clear();
    }
    #endif
  }
}



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