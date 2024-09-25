/*
 * file: pins.h
 * Pin configuration
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
#ifndef PINS_H
#define PINS_H
#include <Arduino.h>

/* Static PIN DEFINTIONS */
#define VOLT_PIN A0 // Define Voltage control
#define AMP_PIN A1  // Define Amperage control

#define ESC_PIN 3 // Define ESC control pin

#define VERNIER_PIN A3 // Define Vernier analog read

/* Button pins D6 D5 D4*/
#define NUM_BUTTONS 3

#define BTN_YELLOW_PIN 6 
#define BTN_GREEN_PIN 5
#define BTN_BLUE_PIN 4

#endif /* PINS_H */