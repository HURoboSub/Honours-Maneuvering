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
 * Date: 18-09-2024
 *
 * Version: 2.1.0
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
#include <Servo.h>      // Servo motor library
#include <TimerEvent.h> // Timer Event Library to use interrupt timer for motor PWM
#include <Wire.h>       // Two Wire Interface Bus (I2C)

/* buttons */
#define NUM_BUTTONS 3

/* MOTOR test parameters */
#define CYCLES 350
#define STEPS 16

#define MTR_STARTUP_DELAY_MS 3000

#define MTR_MIN_CLOCKWISE 1000  // µs for max speed  clockwise
#define MTR_NEUTRAL 1500 // µs rest thrust
#define MTR_MAX_ANTICLOCKWISE 2000 // µs for max speed anticlockwise

#define MTR_INCREMENT 1 // How much the micros should increase each time

#define THRUST_LADDER 50
#define DUR_PROG_A 5
#define DUR_PROG_B 1000
#define DUR_PROG_C 3000
#define WAIT_TIME 50

/* Measurement ADC configuration */
#define MAX_ADC 1023

#define NUM_ADC_READINGS 10

#define MAX_VOLT  16
#define MAX_AMP   10

#define CAL_VOLT  10
#define CAL_AMP   1

/* Vernier properties */ 

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

// BUTTONS
enum buttonIndices{YELLOW, GREEN, BLUE}; // 0 1 2

// enumator for the system states
enum class systemState {
    Setup,       // S0 Init setup state
    Calibrating, // S1
    Reading,     // S2
    Testing,     // S3
    Output       // S4
} currentState; // class storing the current system state

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
    C, // Ramp
    D, // Forward
    E  // Backward
}; 

void CalibrateShunt(void); // Calibrate the shunt for voltage and current
void CalibrateVernier(void); // Calibrate the Vernier force sensor in balance
void selectProgram(void);
void initMotor(void); // Initialise motor
void motorTest(enum testPrograms prog); // Run testprogram on motor
void prog_a_timer_handler(void);
void prog_b_timer_handler(void);
void prog_c_timer_handler(void);
void prog_d_timer_handler(void);
void prog_e_timer_handler(void);

void handleButtons(bool *pState); // Handle button presses and store states in boolean array
void waitforButton(enum buttonIndices btn_i); // wait for single button to be pressed

float readVernier(); // Read Vernier input and return its value
float calcPower(PMEASUREMENT p); // Calculate power and store in measurement structure
void userInterface(systemState cState); // Displays the systemstate on the LCD scren 
// void LCD_show(char **str); // UNUSED single LCD_Show function to handle .clear .cursor .print at once
void output2Serial(PMEASUREMENT p); // outputs the measurement to serial

#endif /* MAIN_H */