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

#include <Wire.h>
#include <LiquidCrystal_I2C.h> // LCD SCREEN
#include <Servo.h>
#include <VernierLib.h> //include Vernier functions
#include <Bounce2.h>    // button debounce lib https://github.com/thomasfredericks/Bounce2/blob/master/examples/more/bounceMore/bounceMore.ino

// buttons
#define NUM_BUTTONS 3

// MOTOR test parameters
#define CYCLES 500
#define STEPS 9
#define MINIMUM_THRUST 1500
#define THRUST_LADDER 50
#define DUR_PROG_A 20
#define DUR_PROG_B 10000
#define THRUST_LADDER 50

// measurement
#define VOLTS_ADC_STEP 20 / 1024
#define AMS_ADC_Step 20 / 1024

struct measurement // structure containing the measurements
{
    float voltage;
    float current;
    float power;
    int force;
};

typedef struct measurement MEASUREMENT;        // MEASUREMENT  == struct
typedef MEASUREMENT        *PMEASUREMENT;      // PMEASUREMENT == struct measurement*

enum testPrograms
{
    A,
    B
}; // testprograms enumerator

void armESC();
int readVernier();
float calcPower(PMEASUREMENT p);
void motorTest(enum testPrograms prog);
void output2Serial(PMEASUREMENT p);

#endif