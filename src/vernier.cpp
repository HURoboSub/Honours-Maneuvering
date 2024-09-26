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

/* EXTERN VARS used in vernier.cpp  */
extern systemState currentState;
extern bool buttonStates[NUM_BUTTONS];
extern PMEASUREMENT pData;    // point to datastrucutre
extern LiquidCrystal_I2C lcd; // set the LCD address to LCD_addr for a LCD_chars by LCD_lines display

float VERNIER_BIAS = 0.0;
float VERNIER_FORCE_BIAS = 0.0;
/*
  Function: calibrateVernier
  Parameters: void
 */
void calibrateVernier(void)
{
  VERNIER_BIAS = 0.0;

  #if defined(LCD) && (LCD == 1)
  lcd.clear();
  lcd.home();                // LCD cursor to 0,0
  lcd.print("CAL vernier!"); // Show instruction on 1 LCD-row
  #endif
  
  /* get 0 N bias until yellow button is pressed*/
  do 
  {
    handleButtons(buttonStates);
    VERNIER_BIAS = middleVernier(); // set Vernier bias to the average 
  } while (buttonStates[YELLOW] == false);

  #ifdef DEBUG_VERNIER
    Serial.println((String)"VERNIER 0N BIAS:\t" + VERNIER_BIAS);
  #endif

  #if defined(LCD) && (LCD == 1)
    lcd.clear();
  #endif

  /* !!HERE A NEW COMMENT IS NEEDED */
  do 
  {
    handleButtons(buttonStates);
    VERNIER_FORCE_BIAS = useTheForce(); // set new force Vernier bias
    delay(10);
  } while (buttonStates[YELLOW] == false);

  #ifdef DEBUG_VERNIER
    Serial.println((String)"VERNIER force bias N BIAS:\t" + VERNIER_FORCE_BIAS);
  #endif
  
  #if defined(LCD) && (LCD == 1)
    lcd.clear();
    lcd.home();               // LCD cursor to 0,0
    lcd.print("Calibrated!"); // Show instruction on 1 LCD-row
    delay(1000);
  #endif
}

/*
  Function: At 0 N, take NUM_ADC_READINGS and returns bias value
  
 */
float middleVernier(void)
{
  float readValue = 0.0;
  float vernierBias = 0.0;

  for (uint8_t i = 0; i < NUM_ADC_READINGS; i++)
  {
    readValue += analogRead(VERNIER_PIN); // Read VERNIER_PIN NUM_ADC_READINGS times and sum it
    delayMicroseconds(10);
  }

  readValue /= NUM_ADC_READINGS; // Calculate average value (total sum / number of readings)

  #if defined(LCD) && (LCD == 1)
    lcd.setCursor(0, 1); // LCD cursor to 0,1
    lcd.print(readValue);
  #endif

  vernierBias = readValue; // set Vernier bias to the average

  return vernierBias;
}


/*
  Function: At 0 N, take NUM_ADC_READINGS and returns bias value
  
 */
float useTheForce(void)
{
  float readSum = 0.0;
  float readAvg = 0.0;

  float voltage = 0.0;
  float force = 0.0;
  float forceBias = 0.0;

  for (uint8_t i = 0; i < NUM_ADC_READINGS; i++)
  {
    // Correct for 0N VERNIER_BIAS
    readSum += analogRead(VERNIER_PIN); // Read VERNIER_PIN NUM_ADC_READINGS times and sum it
    delayMicroseconds(10);
  }

  readAvg = readSum/NUM_ADC_READINGS; // Calculate average value (total sum / number of readings)
  forceBias = readAvg; // set force bias

  readAvg -= VERNIER_BIAS;

  // calc force
  voltage = (readAvg / 1023.0) * 5.0;      // ADC terug naar spanning
  force = voltage * VERNIER_SCALING_FIFTY; // multiply by [N/V]

  #if defined(LCD) && (LCD == 1)
    lcd.home();         // LCD cursor to 0,0
    lcd.print("Force"); // Show instruction on 1 LCD-row
    lcd.setCursor(0, 1); // LCD cursor to 0,1
    lcd.print(force);
    lcd.setCursor(7, 1); // LCD cursor to 0,1
    lcd.print("[N]");
  #endif

  forceBias = readAvg; // set force bias

  #ifdef DEBUG_VERNIER
    Serial.println((String)"VERNIER CAL ADC:\t" + forceBias);
    Serial.println((String)"VERNIER CAL force N:\t" + force);
  #endif

  return forceBias;
}

/*
  Function: Reads vernier sensor and returns force value
  Parameters:
 */
float readVernier()
{
  float readValue = 0.0;
  float voltage = 0.0;
  float force = 0.0;

  currentState = systemState::Reading; // put system to Reading state

  readValue = analogRead(VERNIER_PIN); // single reading

  pData->force_raw = readValue; // set raw value in measurement data

  readValue -= VERNIER_BIAS; // First correct for 0N VERNIER_BIAS
  readValue -= VERNIER_FORCE_BIAS; // Correct for force VERNIER_FORCE_BIAS

  voltage = (readValue / 1023.0) * 5.0;    // ADC terug naar spanning
  force = voltage * VERNIER_SCALING_FIFTY; // multiply by [N/V]

  #ifdef DEBUG_VERNIER
    Serial.print("calibrated force:\t");
    Serial.println(force);
  #endif

  pData->force = force; // set force in structure

  return force; // return calculated force
}