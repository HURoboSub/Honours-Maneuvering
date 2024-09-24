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
 * Date: 24-09-2024
 *
 * Version: 2.3.0
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

void CalibrateVernier(void); // Calibrate the Vernier force sensor in balance
float readVernier(void); // Read Vernier input and return its value

#endif /* VERNIER_H */