/*
 * file: motor.cpp
 *
 * authors:
 *  Jannick Bloemendal
 *  Niels Redegeld
 *  Thijs Vader
 *  Rutger Janssen
 *
 * Hogeschool Utrecht
 * Date: 24-09-2024
 */

#include "main.h"  // Main header
#include "motor.h" // Motor header

/* Motor configuration */
Servo esc; // Create an ESC Servo object

/* Store the state of motor test program */
// program A,       B,       C
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

enum direction_t direction = Forward;

/* Extern variabeles used in motor.cpp  */
extern systemState currentState;
extern bool buttonStates[NUM_BUTTONS];
extern PMEASUREMENT pData;    // point to datastrucutre
extern LiquidCrystal_I2C lcd; // set the LCD address to LCD_addr for a LCD_chars by LCD_lines display

/*
  Function: InitMotor
  Puts motor to MTR_NEUTRAL µs posittion
  Parameters: void
 */
void initMotor(void)
{
  esc.attach(ESC_PIN); // Attach the ESC to the specified pin
  delay(20);

  esc.writeMicroseconds(MTR_NEUTRAL); // Send a signal to the ESC to arm it
  delay(MTR_STARTUP_DELAY_MS);
}

/*
  Function: motorTest
  Deze functie laat de motor door CYCLES standen lopen. DUR_PROG_A, DUR_PROG_B
  Parameters: enum testPrograms prog, which tesprogram to run
  Deps: TimerEvent.h
 */
void motorTest(enum testPrograms prog)
{
  currentState = systemState::Testing; // put system to Testing

#ifdef DEBUG
  Serial.println((String) "Testing motorprogram:" + (int)prog);
#endif

#if defined(LCD) && (LCD == 1)
  lcd.clear();
  lcd.print("Testing ;)");
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
    initMotor();
    timer_motor_test_b.set(DUR_PROG_A, prog_b_timer_handler);

    while (1)
    {
      timer_motor_test_b.update(); // Update the timer

      // Put the vernier sensor read func here (can be another timer if needed)
      readVernier();        // force [N]
      calcPower(pData);     // motor [A] & [V]
      output2Serial(pData); // write data to Serial
    }
    break;

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

      esc.writeMicroseconds(micros_prog_a = micros_prog_a + MTR_INCREMENT);
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
      esc.writeMicroseconds(micros_prog_a = micros_prog_a - MTR_INCREMENT);
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
      esc.writeMicroseconds(micros_prog_a = micros_prog_a + MTR_INCREMENT);
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
  bool *pButtonStates = &buttonStates[0]; // define pointer, pointing to zeroth element of buttonStates array

  switch (direction)
  {
  case Forward:

    if (micros_prog_b == MTR_MAX_ANTICLOCKWISE)
    {
      lcd.clear();
      lcd.home();
      lcd.print("Green for");
      lcd.setCursor(0, 1);
      lcd.print("backwards");

      do
      {
        handleButtons(pButtonStates);
      } while (buttonStates[1] == false); // Wait until green button has been pressed

      direction = Backward; // Change direction
    }

    else
      esc.writeMicroseconds(micros_prog_b = micros_prog_b + MTR_INCREMENT);

    break;

  case Backward:
    if (micros_prog_b == MTR_MIN_CLOCKWISE)
    {
      lcd.clear();
      lcd.home();
      lcd.print("Green for");
      lcd.setCursor(0, 1);
      lcd.print("forwards");

      do
      {
        handleButtons(pButtonStates);
      } while (buttonStates[1] == false); // Wait until green button has been pressed

      direction = Forward;
    }
    else
      esc.writeMicroseconds(micros_prog_b = micros_prog_b - MTR_INCREMENT);
    break;

  default:
    break;
  }
}
