#include <math.h>
#include "complex.h"



void cpl_init(Complex  *num, PRECISION real, PRECISION imag)
{
    num->real = real;
    num->imag = imag;
}

Complex cpl_add(Complex *a, Complex *b)
{
    Complex temp;
    temp.real = a->real + b->real;
    temp.imag = a->imag + b->imag;
    return temp;
}

Complex cpl_mul(Complex *a, Complex *b)
{
    Complex temp;
    temp.real = a->real * b->real - a->imag * b->imag;
    temp.imag = a->real * b->imag + a->imag * b->real;
    return temp;
}

Complex cpl_pow(Complex * base, Complex * power)
{
    PRECISION a, b, c, d, sum1, angle, mag, arg;
    Complex temp;
    a = base->real, b = base->imag, c = power->real, d = power->imag;
    
    arg = atan2(b, a);
    sum1 = a * a + b * b;
    
    mag = pow(sum1, c / 2) * exp(-d * arg);
    
    if( sum1 == 0)
    {
        temp.imag = 0;
        temp.real = 0;
    }
    else
    {   
        angle = c * arg + .5 * d * log(mag);
        
        temp.real = mag * cos(angle);
        temp.imag = mag * sin(angle);
    }
   

    
    return temp;
}



PRECISION cpl_mag(Complex* a)
{
    return sqrt( a->real*a->real + a->imag * a->imag);
}











