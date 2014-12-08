#include "Calculate.h"

double multi(tv * a, tv * b)
{
    double temp1, temp2, temp3;
    temp1 = a->tv_usec;
    temp2 = b->tv_usec;
    temp1 += a->tv_sec * 96000000;
    temp2 += b->tv_sec * 96000000;
    temp3 = temp1 * temp2;
    return temp3;
}

double sumup(tv * a)
{
    double temp1;
    temp1 = a->tv_usec;
    temp1 += a->tv_sec * 96000000;
    return temp1;    
}

double LinearRegression(coeff * co)
{
    double temp1 = 0;
    double temp2 = 0;
    double temp3 = 0;
    double temp4 = 0;
    double Lxy, Lxx;
    for (int i=0; i<10; i++)
    {
        temp1 += multi(& co->x[i], & co->y[i]);
        temp2 += sumup(& co->x[i]);
        temp3 += sumup(& co->y[i]);
        temp4 += multi(&co->x[i], &co->x[i]);
    }
    temp3 = temp2*temp3;
    temp3 = temp3/10;
    Lxy = (temp1 - temp3);
    temp2 = temp2 * temp2;
    temp2 = temp2/10;
    Lxx = (temp4 - temp2);
    double drift;
    drift = Lxy/Lxx;
    return drift;    
}