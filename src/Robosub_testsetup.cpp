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

/* Timing configuration */
const int timeBtwnReadings = 500; // time between Vernierr
unsigned long lastReadTime = 0ul;

/* LCD properties */ 
const uint8_t LCD_addr = 0x3f;  // i2c-address of LCD screen
const uint8_t LCD_cols = 16;    // number of chars on lcd screen
const uint8_t LCD_rows = 2;     // number of lines

LiquidCrystal_I2C lcd(LCD_addr, LCD_cols, LCD_rows); // set the LCD address to LCD_addr for a LCD_chars by LCD_lines display

VernierLib Vernier; // create an instance of the VernierLib library

/* PIN DEFINTIONS */
uint8_t VOLT_PIN = A0; // Define Voltage control
uint8_t AMP_PIN = A1;  // Define Amperage control

uint8_t ESC_PIN = 11;                // Define ESC control pin

TimerEvent timer_motor_test_a;
TimerEvent timer_motor_test_b;
TimerEvent timer_motor_test_c;
// Create the timers

uint8_t timer_expired = 0;
bool continuous_motor_test = true;

const uint8_t BUTTON_PINS[NUM_BUTTONS] = {4, 7, 8}; // Define ESC control pin D4 D7 D8
Bounce * buttons = new Bounce[NUM_BUTTONS]; // Initiate 3 Bounce objects
bool buttonStates[NUM_BUTTONS] = {false}; // bool array storing the buttonStates
bool* pButtonStates = &buttonStates[0]; // define pointer, pointing to zeroth element of buttonStates array

Servo esc; // Create a Servo object

MEASUREMENT data; // measurement data
PMEASUREMENT pData = &data; // point to datastrucutre

enum testPrograms testProgram = B; // default to test program A

void setup()
{
  currentState = systemState::Setup; // put system to Setup state

  Serial.begin(9600);           // initialize serial communication at 9600 bits per second:
  Serial.println("Program starting..");

  pinMode(LED_BUILTIN, OUTPUT); // specifies that LED_BUILTIN will be used for output
  pinMode(ESC_PIN, OUTPUT);

  pinMode(VOLT_PIN, INPUT);
  pinMode(AMP_PIN, INPUT);

  pinMode(BUTTON_PINS[0], INPUT_PULLUP);
  pinMode(BUTTON_PINS[1], INPUT_PULLUP);
  pinMode(BUTTON_PINS[2], INPUT_PULLUP);

  // After setting up the button, setup debouncer
  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].attach( BUTTON_PINS[i] , INPUT_PULLUP); //setup the bounce instance for the current button
    buttons[i].interval(25);                           // debounce interval in ms
  }


  // measuremunt datastructure
  output2Serial(pData); // output header row to serial

  lcd.init(); // initialize the lcd  screen
  lcd.backlight();
  lcd.print("Starting...");
  // https://github.com/YukiSakuma/arduino/blob/a0d36da69587d03019de49ea383efab30b5f0fac/VernierAnalogAutoID/VernierAnalogAutoID.ino#L74C1-L74C102
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

  // motor
  esc.attach(ESC_PIN);  // Attach the ESC to the specified pin
  initMotor();           // Initialize the ESC
}

void loop()
{
  lastReadTime = millis();
  handleButtons(pButtonStates); 

  pData->force = readVernier();
  calcPower(pData);
  motorTest(testProgram);
  output2Serial(pData);

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
void handleButtons(bool *pState)
{
  currentState = systemState::Reading; // put system to Reading state

  // for the NUM_BUTTONS increase i and state pointer
  for (int i = 0; i < NUM_BUTTONS; pState++, i++)
  {
    // Update the Bounce instance :
    buttons[i].update();
    // If it fell, flag the need to toggle the LED
    if (buttons[i].fell())
    {
      // aBtnPressed = true;
      *pState = buttons[i].fell(); // change left value of this button state
      digitalWrite(LED_BUILTIN, HIGH);
    }
  }
}

/*
  Function: Reads varnier sensor and returns value
  Parameters:
 */
int readVernier()
{
  currentState = systemState::Reading; // put system to Reading state

  float sensorReading = Vernier.readSensor();
  delay(timeBtwnReadings); // stabilize time between readings (!!improve FUTURE maybe timer?)

  return sensorReading;
}

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

  // Read analog values from pins
  voltVal = analogRead(VOLT_PIN); // Read voltage value
  ampVal = analogRead(AMP_PIN);   // Read current value

  // Convert analog values to actual voltage and current
  p->voltage = (voltVal * VOLTS_ADC_STEP); // Calculate voltage in volts
  p->current = (ampVal * AMS_ADC_STEP);    // Calculate current in amperes

  power = p->voltage * p->current;  // Calculate power using the formula: power = voltage * current

  p->power = power;   // Store calculated power in the measurement structure

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

    Serial.print(millis() - lastReadTime);
    Serial.print(",");
    Serial.println(p->force);
    Serial.print(",");
    Serial.print(p->voltage, 2);
    Serial.print(",");
    Serial.print(p->current, 2);
    Serial.print(",");
    Serial.print(p->power, 2);
  }
}

/*
Deze functie laat de motor door 9 standen lopen, van 1550 tot 2000. duurt intotaal 90 seconden
*/
void motorTest(enum testPrograms prog)
{

  uint8_t i;
  uint8_t thrust = 50;

  currentState = systemState::Testing; // put system to Testing 

  switch (prog)
  {
  case A:
    timer_motor_test_a.set(DUR_PROG_A, prog_a_timer_handler); // Set the timer
    /* Testprogramma A continuous
          Laat de motor continue harder draaien, duurt DUR_PROG_A msecs*/
    continuous_motor_test = true;  
    while(continuous_motor_test) // While loop gets played as long as continuous_motor_test is true
    {
        timer_motor_test_a.update();  // Update the timer 
        // Put the vernier sensor read func here (can be another timer if needed)
        if(timer_expired >= CYCLES )  // Check if the loop has been played 500 times
        {
          continuous_motor_test = false;  // Set the bool to false to stop the while loop
          timer_expired = 0;  // Reset timer_expired
          esc.writeMicroseconds(MINIMUM_THRUST);  // Set the motor to 0 RPM
        }
    }

    break;

  case B:
   timer_motor_test_b.set(DUR_PROG_B, prog_b_timer_handler);  // Set the timer
    /* Testprogramma B ladder 
       Deze functie laat de motor door 9 standen lopen, van 1550 tot 2000. duurt intotaal 90 seconden 
     */
    continuous_motor_test = true;
    while(continuous_motor_test)  // While loop gets played as long as continuous_motor_test is true
    {
        timer_motor_test_b.update();  // Update the timer
        // Put the vernier sensor read func here (can be another timer if needed)
        if(timer_expired >= STEPS)  //Check if the loop has been played 9 times
        {
          continuous_motor_test = false;  //Set bool to false to stop loop
          timer_expired = 0;  // Reset timer_expired
          esc.writeMicroseconds(MINIMUM_THRUST);  // Set the motor to 0 RPM
        }
    }
    break;

    case C:
     timer_motor_test_c.set(DUR_PROG_C, prog_c_timer_handler);  // Set the timer
    /*Testprogramma C Ramp
      Deze functie laat de motor direct op fullspeed gaan
    */
    timer_motor_test_c.update();  // Update the timer
        // Put the vernier sensor read func here (can be another timer if needed)
        if(timer_expired >= 1)  //Check if the loop has been played 9 times
        {
          continuous_motor_test = false;  //Set bool to false to stop loop
          timer_expired = 0;  // Reset timer_expired
          esc.writeMicroseconds(MINIMUM_THRUST);  // Set the motor to 0 RPM
        }
    break;

  default:
    #ifdef DEBUG
      Serial.println("Verkeerd motor test programma doorgegeven");
    #endif
    break;
  }
}

void prog_a_timer_handler(void)
{
  uint8_t i;
  uint8_t thrust = 50;

        thrust = thrust + 1;  	                        // Ramp the thrust up with one
        esc.writeMicroseconds(MINIMUM_THRUST + thrust); // Set motor to 1500+ thrust

        timer_expired += 1;                             // Add one to timer_expired
}

void prog_b_timer_handler(void)
{
  uint8_t i;
  uint8_t thrust = 50;
  
    esc.writeMicroseconds(MINIMUM_THRUST + thrust);     // Set motor to 1500 + thrust
    thrust = thrust + THRUST_LADDER;                    // Add 50 to thrust 
    timer_expired += 1;                                 // Add one to timer_expired
}

void prog_c_timer_handler(void)
{ 
    esc.writeMicroseconds(MAXIMUM_THRUST);              // Set motor to 2000
    timer_expired += 1;                                 // Add one to timer_expired
}

/*
  Function: userInterface
    Handles different system states on the LCD screen
  Parameters: class enumator with the current State
 */
void userInterface(systemState cState)
{
  switch (currentState)
  {
  case systemState::Setup:
    // Setup state here
    lcd.setCursor(0, 0);
    lcd.print("S: Setup");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    break;
  case systemState::Calibrating:
    // Calibrating state here
    lcd.setCursor(0, 0);
    lcd.print("S: Calibrating");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    break;
  case systemState::Reading:
    // S2 here
    lcd.setCursor(0, 0);
    lcd.print("S: Reading");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    break;
  case systemState::Testing:
    // S3 here
    lcd.setCursor(0, 0);
    lcd.print("S: Testing");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    break;
  case systemState::Output:
    // S4 here
    lcd.setCursor(0, 0);
    lcd.print("S: Output");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    break;

  default:
    break;
  }
}