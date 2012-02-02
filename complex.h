#ifndef COMPLEX_H
#define COMPLEX_H

#ifndef PRECISION
#define PRECISION double
#endif

typedef struct 
{
    PRECISION real;
    PRECISION imag;
} Complex;

void cpl_init(Complex  *num, PRECISION real, PRECISION imag);
Complex cpl_add(Complex *a, Complex *b);
Complex cpl_mul(Complex *a, Complex *b);
PRECISION cpl_mag(Complex* a);
Complex cpl_pow(Complex * base, Complex * power);

#endif

