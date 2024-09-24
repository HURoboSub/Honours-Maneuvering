/*
 * file: vernier.cpp
 *
 * Source file related to calibrating and reading the vernier force sensor
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

/* Extern variabeles used in vernier.cpp  */
extern systemState currentState;
extern bool buttonStates[NUM_BUTTONS];
extern PMEASUREMENT pData;    // point to datastrucutre
extern LiquidCrystal_I2C lcd; // set the LCD address to LCD_addr for a LCD_chars by LCD_lines display

/* Vernier */
float VERNIER_BIAS = 550.0;

/*
  Function: CalibrateVernier
  Parameters: void
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

  waitforButton(YELLOW); // Wait until yellow button has been pressed

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
  Function: Reads varnier sensor and returns value
  Parameters:
 */
float readVernier(void)
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
  force = voltage * VERNIER_SCALING_FIFTY; // multiply by [N/V]

  // #ifdef DEBUG_VERNIER
  // Serial.print("voltage vernier:\t");
  // Serial.println(voltage);
  // #endif

  pData->force = force; // set force in structure

  return force; // return calculated force
}