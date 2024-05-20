#include "unity.h"

#include "main.h" // main header file

#define DEBUG // (Serial) DEBUG mode (un)comment to toggle

/* Timing configuration */
const int timeBtwnReadings = 500; // time between Vernierr
unsigned long lastReadTime = 0ul;

LiquidCrystal_I2C lcd(LCD_addr, LCD_COLS, LCD_ROWS); // set the LCD address to LCD_addr for a LCD_chars by LCD_lines display

char const *rowOneLCD = "               ";   // const pointer to a char array containing linezero of LCD
char const *rowTwoLCD = "               "; // textrow one of LCD

VernierLib Vernier; // create an instance of the VernierLib library

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

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
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
  unsigned char y; // y loop index

  // Memory allocation for the display text
  char **dispText = new char *[LCD_ROWS];

  for (y = 0; y < LCD_ROWS; y++)
  {
    dispText[y] = new char[LCD_COLS];
  }

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

  // Show the updated display
  LCD_show(dispText);

  // delete allocated memory for dispText buffer 
  for (y = 0; y < LCD_ROWS; y++)
  {
    delete[] dispText[y];
  }
  delete[] dispText;

}

void test_states(void) {
    // test each state
    for (int state = 0; state < 5; state++)
    {
        currentState = static_cast<systemState>(state);
        userInterface(currentState);
    }
}

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(test_states);
  return UNITY_END();
}

// WARNING!!! PLEASE REMOVE UNNECESSARY MAIN IMPLEMENTATIONS //

/**
  * For native dev-platform or for some embedded frameworks
  */
int main(void) {
  return runUnityTests();
}

/**
  * For Arduino framework
  */
void setup() {
  // Wait ~2 seconds before the Unity test runner
  // establishes connection with a board Serial interface
  delay(2000);

  runUnityTests();
}
void loop() {
    
}
