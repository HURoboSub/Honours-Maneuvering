/*
 * file: main.h
 * Header file
 *
 * authors:
 *  Jannick Bloemendal
 *  Niels Redegeld
 *  Thijs Vader
 *  Rutger Jansen
 *
 * Hogeschool Utrecht
 * Date: 24-09-2024
 *
 * Version: 2.3.0
 *
 * CHANGELOG:
 *
 *
 */

#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <Bounce2.h>    // button debounce lib https://github.com/thomasfredericks/Bounce2/blob/master/examples/more/bounceMore/bounceMore.ino
#include <LiquidCrystal_I2C.h> // LCD i2c screen

#include <Wire.h>       // Two Wire Interface Bus (I2C)

#include "motor.h" // Motor header
#include "pins.h"
#include "vernier.h"

/* Measurement ADC configuration */
#define MAX_ADC 1023

#define NUM_ADC_READINGS 10

#define MAX_VOLT  16
#define MAX_AMP   10

#define CAL_VOLT  10
#define CAL_AMP   1

/* LCD properties */ 
#define LCD_addr 0x27  // i2c-address of LCD screen
#define LCD_COLS 16    // number of chars on lcd screen
#define LCD_ROWS 2    // number of lines

struct measurement // structure containing the measurements
{
    float voltage; // SHUNT
    float current; // SHUNT
    float power; // SHUNT
    float force; // VERNIER force
    int force_raw; // VERNIER raw ADC value
};

typedef struct measurement MEASUREMENT;     // MEASUREMENT  == struct
typedef MEASUREMENT       *PMEASUREMENT;    // PMEASUREMENT == struct measurement*

// enumator for the system states
enum class systemState {
    Setup,       // S0 Init setup state
    Calibrating, // S1
    Reading,     // S2
    Testing,     // S3
    Output       // S4
};

/* test states program A*/
#define NEUTRAL 0
#define ADDING 1
#define SUBTRACTING 2
#define ADDING_HALVE 3

/* test states program B*/
#define UPPER 1
#define LOWER 2
#define FINISHED 3

enum buttonIndices{YELLOW, GREEN, BLUE}; // 0 1 2

void CalibrateShunt(void); // Calibrate the shunt for voltage and current
testPrograms selectProgram(void);

void handleButtons(bool *pState); // Handle button presses and store states in boolean array
void waitforButton(enum buttonIndices btn_i); // wait for single button to be pressed

float calcPower(PMEASUREMENT p); // Calculate power and store in measurement structure

// void LCD_show(char **str); // UNUSED single LCD_Show function to handle .clear .cursor .print at once
void userInterface(systemState cState); // Displays the systemstate on the LCD scren 
void output2Serial(PMEASUREMENT p); // outputs the measurement to serial

#endif /* MAIN_H */