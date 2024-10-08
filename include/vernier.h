/*
 * file: vernier.h
 * Vernier force sensor header
 *
 * authors:
 *  Jannick Bloemendal
 *  Niels Redegeld
 *  Thijs Vader
 *  Rutger Jansen
 *
 * Hogeschool Utrecht
 * Date: 08-10-2024
 *
 * Version: 2.4.0
 *
 * CHANGELOG:
 *
 *
 */

#ifndef VERNIER_H
#define VERNIER_H

/* Vernier properties */ 
/* Scaling factors for the two different Vernier modes  */
#define VERNIER_SCALING_TEN 4.67 // [0 - 10 N]
#define VERNIER_SCALING_FIFTY 23.45 //[0 - 50 N]

// how many times / second should the reading on the display change
// WARNING: THIS IS NOT COMPLETELY ACCURATE
#define VERNIER_CAL_UPDATE_RATE 10

void calibrateVernier(void); // Calibrate the Vernier force sensor in balance
float middleVernier(void);

float useTheForce(void);

float readVernier(); // Read Vernier input and return its value

#endif /* VERNIER_H */