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

/* Measurement ADC configuration */
#define MAX_ADC 1023

#define NUM_ADC_READINGS 10

#define MAX_VOLT  16
#define MAX_AMP   10

#define CAL_VOLT  10
#define CAL_AMP   1

/* Vernier properties */ 
/* Scaling factors for the two different Vernier modes  */
#define VERNIER_SCALING_TEN 4.67 // [0 - 10 N]
#define VERNIER_SCALING_FIFTY 23.45 //[0 - 50 N]

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
    int force_raw; // // VERNIER raw ADC value
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

enum testPhasesC
{
    First,
    Second,
    Third,
    Fourth
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

enum testPrograms // Motor test programs
{
    A, // Continuos
    B, // Ladder
    NUM_PROGRAMS
}; 

enum buttonIndices{YELLOW, GREEN, BLUE}; // 0 1 2

enum direction_t
{
    Forward,
    Backward
};

void CalibrateShunt(void); // Calibrate the shunt for voltage and current
void CalibrateVernier(void); // Calibrate the Vernier force sensor in balance
testPrograms selectProgram(void);

void handleButtons(bool *pState); // Handle button presses and store states in boolean array
void waitforButton(enum buttonIndices btn_i); // wait for single button to be pressed

float readVernier(void); // Read Vernier input and return its value
float calcPower(PMEASUREMENT p); // Calculate power and store in measurement structure
void userInterface(systemState cState); // Displays the systemstate on the LCD scren 
// void LCD_show(char **str); // UNUSED single LCD_Show function to handle .clear .cursor .print at once
void output2Serial(PMEASUREMENT p); // outputs the measurement to serial

#endif /* MAIN_H */