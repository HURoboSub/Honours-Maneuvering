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
#include <Bounce2.h> // button debounce lib
// UART serialplotter lib?

// buttons
#define NUM_BTNS 3 

#define CYCLES 500
#define STEPS 9
#define MINIMUM_THRUST 1500
#define THRUST_LADDER  50


void armESC();
int readVernier();
void motor_test_ladder(void);
void motor_test_continuous(void);

#endif