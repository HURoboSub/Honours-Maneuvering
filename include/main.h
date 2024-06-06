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
#include <TimerEvent.h> // Timer Event Library to use interrupt timer for motor PWM
#include <Bounce2.h>    // button debounce lib https://github.com/thomasfredericks/Bounce2/blob/master/examples/more/bounceMore/bounceMore.ino

/* buttons */
#define NUM_BUTTONS 3

/* MOTOR test parameters */
#define CYCLES 500
#define STEPS 9
<<<<<<< HEAD

#define MTR_STARTUP_DELAY_MS 5000

#define MTR_MIN_CLOCKWISE 1000  // µs for max speed  clockwise
#define MTR_NEUTRAL 1500 // µs rest thrust
#define MTR_MAX_ANTICLOCKWISE 2250 // µs for max speed anticlockwise

#define MRT_INCREMENT 2 // how much the micros should increase each time

=======
#define MINIMUM_THRUST 1500 // rest thrust
#define MAXIMUM_THRUST 2250
>>>>>>> origin/main
#define THRUST_LADDER 50
#define DUR_PROG_A 20
#define DUR_PROG_B 10000
#define DUR_PROG_C 3000
<<<<<<< HEAD
=======
#define THRUST_LADDER 20
>>>>>>> origin/main

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
    B, // Ladder
    C  // Ramp?
}; 

void Calibrate(void); // Calibrate the shunt for voltage and current

void initMotor(void); // Initialise motor
void motorTest(enum testPrograms prog); // Run testprogram on motor
void prog_a_timer_handler(void);
void prog_b_timer_handler(void);
void prog_c_timer_handler(void);

void handleButtons(bool *pState); // Handle button presses and store states in boolean array
int readVernier(); // read Vernier input and return its value
float calcPower(PMEASUREMENT p); // calculate power and store in measurement structure
void userInterface(systemState cState); // Displays the systemstate on the LCD scren 
// void LCD_show(char **str); // UNUSED single LCD_Show function to handle .clear .cursor .print at once
void output2Serial(PMEASUREMENT p); // outputs the measurement to serial

#endif