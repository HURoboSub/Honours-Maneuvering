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
// #define DEBUG_MOTOR
#define CAL_VERNIER
#define CAL_SHUNT// Whether to calibrate shunt at the beginning

#define LCD 1
#define DEBUG_VERNIER
// #define USE_VERNIERLIB

enum testPrograms testProgram = A; // which testprogram to run  program A

/* PIN DEFINTIONS */
uint8_t VOLT_PIN = A0; // Define Voltage control
uint8_t AMP_PIN = A1; // Define Amperage control

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
float VENIER_BIAS = 550.0;

LiquidCrystal_I2C lcd(LCD_addr, LCD_COLS, LCD_ROWS); // set the LCD address to LCD_addr for a LCD_chars by LCD_lines display

char const *rowOneLCD = "               "; // const pointer to a char array containing linezero of LCD
char const *rowTwoLCD = "               "; // textrow one of LCD

/* Motor configuration */
Servo esc; // Create a Servo object

// Store the state of motor test program
                           // prog A, B, C
uint8_t motorTestState[3] = {NEUTRAL, NEUTRAL, 0};

// Create the 3 timers for each motortestprogram
TimerEvent timer_motor_test_a;
TimerEvent timer_motor_test_b;
TimerEvent timer_motor_test_c;

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
}

void loop()
{
  float testVal = 0.0;
  
  testVal = (float) analogRead(AMP_PIN);   // Read current value

  #ifdef DEBUG
  Serial.println((String)"raw testVal:" +testVal);
  #endif

}


/*
  Function: calcPower
  Calculate power based on voltage and current measurements

  Parameters: PMEASUREMENT p, pointer to measurement data structure
 */
float calcPower(PMEASUREMENT p)
{
  float power = 0.0; // Initialize power variable
  float ampVal = 0.0;  // Initialize analog value for current
  float voltVal = 0.0; // Initialize analog value for voltage

  // Read analog values from pins
  voltVal = (float) analogRead(VOLT_PIN); // Read voltage value
  delay(20);
  ampVal = (float) analogRead(AMP_PIN);   // Read current value

  // Convert analog values to actual voltage and current
  p->voltage = (voltVal * ADC_V_Step); // Calculate voltage in volts
  p->current = (ampVal * ADC_A_Step);  // Calculate current in amperes

  power = p->voltage * p->current; // Calculate power using the formula: power = voltage * current

  p->power = power; // Store calculated power in the measurement structure

  return power; // Return the calculated power
}