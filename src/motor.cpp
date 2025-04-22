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
 * Date: 08-10-2024
 */

#include "main.h"  // Main header
#include "motor.h" // Motor header

/* EXTERN VARS used in motor.cpp  */
extern systemState currentState;
extern bool buttonStates[NUM_BUTTONS];
extern PMEASUREMENT pData;    // point to datastrucutre
extern LiquidCrystal_I2C lcd; // set the LCD address to LCD_addr for a LCD_chars by LCD_lines display

/* Motor configuration */
Servo esc; // Create an ESC Servo object

/* Store the state of motor test program */
// program A,       B,       C
uint8_t motorTestState[3] = {NEUTRAL, NEUTRAL, ADDING};

// Create the 3 timers for each motortestprogram
TimerEvent timer_motor_test;

uint8_t timer_expired = 0;
bool continuous_motor_test = true;

uint32_t wait = 0;

uint32_t micros_prog = MTR_NEUTRAL;

enum direction_t direction = Forward; // Start  with forward

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
  Serial.println((String) "Testing motorprogram: " + (int)prog);
#endif

#if defined(LCD) && (LCD == 1)
  lcd.clear();
  lcd.print("Testing ");
#endif

  switch (prog)
  {
  case A:
    // initMotor();
    timer_motor_test.set(DUR_PROG_A, prog_a_timer_handler); // Set the timer
    //       Laat de motor continue harder draaien, duurt DUR_PROG_A msecs*/
    continuous_motor_test = true;
    while (continuous_motor_test) // While loop gets played as long as continuous_motor_test is true
    {
      timer_motor_test.update(); // Update the timer

      // Put the vernier sensor read func here (can be another timer if needed)
      readVernier();        // force [N]
      // calcPower(pData);     // motor [A] & [V]
      output2Serial(pData); // write data to Serial

      if (timer_expired >= CYCLES) // Check if the loop has been played 500 times
      {
        continuous_motor_test = false;      // Set the bool to false to stop the while loop
        timer_expired = 0;                  // Reset timer_expired
        esc.writeMicroseconds(MTR_NEUTRAL); // Set the motor to 0 RPM
      }
    }

    break; /* Program A */
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

    if (micros_prog >= MTR_MAX_ANTICLOCKWISE)
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

      esc.writeMicroseconds(micros_prog = micros_prog + MTR_INCREMENT);
  #ifdef DEBUG_MOTOR
      Serial.println((String) "micros_prog = " + micros_prog);
  #endif
    }
    break; // End of ADDING

  case SUBTRACTING: // 2
  #ifdef DEBUG_MOTOR
    Serial.println("Reached case Subtracting");
  #endif

    if (micros_prog == MTR_MIN_CLOCKWISE)
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
      esc.writeMicroseconds(micros_prog = micros_prog - MTR_INCREMENT);
    }
    break; // End of subtracting

  case ADDING_HALVE: // 3
  #ifdef DEBUG_MOTOR
    Serial.println("Reached case Adding_halve");
  #endif
    if (micros_prog == MTR_NEUTRAL)
    {
      wait += 1;
      if (wait >= WAIT_TIME)
      {
        motorTestState[A] = NEUTRAL;
        wait = 0;
      }
    }
    else
      esc.writeMicroseconds(micros_prog = micros_prog + MTR_INCREMENT);
    break; // End of ADDING_HALVE

  default:
    esc.writeMicroseconds(MTR_NEUTRAL);
    break;
  }
  timer_expired = timer_expired + 1;
  pData->micros = micros_prog; // copy to structu
}
