/*
 * file: motor.h
 * Motor header file
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
#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>
#include "main.h"
#include "pins.h"

#include <Servo.h>      // Servo motor library
#include <TimerEvent.h> // Timer Event Library to use interrupt timer for motor PWM]

/* MOTOR test parameters */
#define CYCLES 350
#define STEPS 16

#define MTR_STARTUP_DELAY_MS 3000

#define MTR_MIN_CLOCKWISE 1000     // µs for max speed  clockwise
#define MTR_NEUTRAL 1500           // µs rest thrust
#define MTR_MAX_ANTICLOCKWISE 2000 // µs for max speed anticlockwise

#define MTR_INCREMENT 1 // How much the micros should increase each time

#define THRUST_LADDER 50
#define DUR_PROG_A 5
#define DUR_PROG_B 1000
#define DUR_PROG_C 3000
#define WAIT_TIME 50

enum direction_t
{
    Forward,
    Backward
};

void initMotor(void);                   // Initialise motor
void motorTest(enum testPrograms prog); // Run testprogram on motor
void prog_a_timer_handler(void);
void prog_b_timer_handler(void);

#endif /* MOTOR_H */