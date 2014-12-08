#ifndef CALCULATE_H
#define CALCULATE_H

#include "mbed.h"

typedef struct timeval {

    unsigned int tv_sec;

    unsigned int tv_usec;

} tv;

typedef struct coeffient {
    
    int Offset_sec;
    
    int Offset_usec;
    
    int off_persec;
    
    double Drift;
    
    tv y [10];

    tv x [10];

} coeff;    

double multi(tv * a, tv * b);
double LinearRegression(coeff * co);
double sumup(tv * a);

#endif