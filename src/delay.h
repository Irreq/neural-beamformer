#define AVX

#if defined AVX
#include <immintrin.h>
#endif

void delay_naive(float *signal, float *h, float *out);

void py_wrapper(double azimuth,
                double elevation,
                int columns,
                int rows,
                float distance,
                float fs,
                float propagation_speed,
                float *coefficients);

#ifdef AVX
void delay_vectorized(float *signal, float *h, float *out);
#endif

void directional_antenna_delay_coefficients(double azimuth,
                                            double elevation,
                                            int columns,
                                            int rows,
                                            float distance,
                                            float fs,
                                            float propagation_speed,
                                            float **coefficients);