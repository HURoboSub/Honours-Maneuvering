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
 * Date: 29-04-2024
 *
 * Version: 1.1.0
 *
 * CHANGELOG:
 *
 *
 */

#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>

#include <Wire.h>       // Two Wire Interface Bus (I2C)
#include <LiquidCrystal_I2C.h> // LCD i2c screen
#include <Servo.h>      // Servo motor library
#include <Bounce2.h>    // button debounce lib https://github.com/thomasfredericks/Bounce2/blob/master/examples/more/bounceMore/bounceMore.ino

/* buttons */
#define NUM_BUTTONS 3

/* MOTOR test parameters */
#define CYCLES 500
#define STEPS 9
#define MINIMUM_THRUST 1500
#define THRUST_LADDER 50
#define DUR_PROG_A 20
#define DUR_PROG_B 10000
#define THRUST_LADDER 50

/* Measurement ADC conf */
#define MAX_ADC 1023

#define NUM_ADC_READINGS 10

#define MAX_VOLT  16
#define MAX_AMP   10

#define CAL_VOLT  10
#define CAL_AMP   1

/* LCD properties */ 
#define LCD_addr 0x3f  // i2c-address of LCD screen
#define LCD_COLS 16    // number of chars on lcd screen
#define LCD_ROWS 2    // number of lines

struct measurement // structure containing the measurements
{
    float voltage;
    float current;
    float power;
    int force;
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
} currentState; // class storing the current system state

enum testPrograms // Motor test programm
{
    A, // Continuos
    B  // Ladder
}; 
void Calibrate();
void initMotor(); // Initialise motor
void motorTest(enum testPrograms prog); // Run testprogram on motor
void handleButtons(bool *pState); // Handle button presses and store states in boolean array
int readVernier(); // read Vernier input and return its value
float calcPower(PMEASUREMENT p); // calculate power and store in measurement structure
void userInterface(systemState cState); // Displays the systemstate on the LCD scren 
void LCD_show(char **str);
void output2Serial(PMEASUREMENT p); // outputs the measurement to serial

#endif