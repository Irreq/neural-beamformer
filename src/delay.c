/******************************************************************************
 * Title                 :   Delay signals
 * Filename              :   delay.c
 * Author                :   Irreq
 * Origin Date           :   10/11/2022
 * Version               :   1.0.0
 * Compiler              :   gcc (GCC) 12.2.0
 * Target                :   x86_64 GNU/Linux
 * Notes                 :   None
 ******************************************************************************
 
 Functions to calculate filter coefficients and two functions to perform the
 delay for the delay-and-sum beamformer. 
 
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "delay.h"

#define N_TAPS 256 // 16
#define OFFSET N_TAPS / 2

#define PI 3.14159265359

#define N_SAMPLES 512 // 65536 // 2^16

/*
Delay a signal by creating a reversed convolution of size (N + M)
and return only of size N
*/
void delay_naive(float *signal, float *h, float *out)
{
    float *padded = malloc((N_SAMPLES + N_TAPS) * sizeof(float));

    // Zero entire buffer
    for (int i = 0; i < N_SAMPLES + N_TAPS; i++)
    {
        padded[i] = 0.0;
    }

    // Load offset signal to buffer
    for (int i = 0; i < N_SAMPLES; i++)
    {
        padded[i + OFFSET] = signal[i];
    }

    // 1D backwards convolution
    for (int i = 0; i < N_SAMPLES; i++)
    {
        out[i] = 0.0;

        for (int k = 0; k < N_TAPS; k++)
        {
            out[i] += h[k] * padded[i + k];
        }
    }

    free(padded);
}

/*
Python wrapper used to initate the functions
*/
void py_wrapper(double azimuth,   // Horizontal
                double elevation, // Vertical
                int columns,
                int rows,
                float distance, // Distance between elements
                float fs,
                float propagation_speed,
                float *coefficients)
{

    float **tmp_coefficients = (float **)malloc((columns * rows) * sizeof(float *));

    for (int i = 0; i < columns * rows; i++)
    {
        tmp_coefficients[i] = (float *)malloc(N_TAPS * sizeof(float));
    }

    directional_antenna_delay_coefficients(azimuth,   // Horizontal
                                           elevation, // Vertical
                                           columns,
                                           rows,
                                           distance, // Distance between elements
                                           fs,
                                           propagation_speed,
                                           tmp_coefficients);

    for (int element = 0; element < columns * rows; element++)
    {
        for (int tap = 0; tap < N_TAPS; tap++)
        {
            coefficients[element * N_TAPS + tap] = tmp_coefficients[element][tap];
        }
    }

    for (int i = 0; i < columns * rows; i++)
    {
        free(tmp_coefficients[i]);
    }

    free(tmp_coefficients);
}

#ifdef AVX // Simd definitions
#define SSE_SIMD_LENGTH 4
#define AVX_SIMD_LENGTH 8

#define KERNEL_LENGTH 16
#define VECTOR_LENGTH 16
#define ALIGNMENT 32 // Must be divisible by 32

/*
Delay a signal by creating a reversed convolution of size (N + M)
and return only of size N using the immintrin AVX instructions
using unrolled Fused Multiply Addition (FMA)
*/
void delay_vectorized(float *signal, float *h, float *out)
{
    // Allign data
    __m256 aligned_kernel[KERNEL_LENGTH] __attribute__((aligned(ALIGNMENT)));
    __m256 data_block __attribute__((aligned(ALIGNMENT)));

    // Two accumulators are set up
    __m256 accumulator0 __attribute__((aligned(ALIGNMENT)));
    __m256 accumulator1 __attribute__((aligned(ALIGNMENT)));

    // Repeat the kernel across the vector
    for (int i = 0; i < KERNEL_LENGTH; i++)
    {
        aligned_kernel[i] = _mm256_broadcast_ss(&h[i]);
    }

    // Allocate memory for the padded signal
    float *padded = malloc((N_SAMPLES + N_TAPS) * sizeof(float));

    // Zero entire buffer
    for (int i = 0; i < N_SAMPLES + N_TAPS; i++)
    {
        padded[i] = 0.0;
    }

    // Load signal to buffer
    for (int i = 0; i < N_SAMPLES; i++)
    {
        padded[i + OFFSET] = signal[i];
    }

    for (int i = 0; i < N_SAMPLES; i += VECTOR_LENGTH)
    {
        accumulator0 = _mm256_setzero_ps();
        accumulator1 = _mm256_setzero_ps();

        for (int k = 0; k < KERNEL_LENGTH; k += VECTOR_LENGTH)
        {
            int data_offset = i + k;

            for (int l = 0; l < SSE_SIMD_LENGTH; l++)
            {

                for (int m = 0; m < VECTOR_LENGTH; m += SSE_SIMD_LENGTH)
                {
                    // First block
                    data_block = _mm256_loadu_ps(padded + l + data_offset + m);

                    accumulator0 = _mm256_fmadd_ps(
                        aligned_kernel[k + l + m], data_block, accumulator0);

                    // Second block
                    data_block = _mm256_loadu_ps(padded + l + data_offset + m + AVX_SIMD_LENGTH);

                    accumulator1 = _mm256_fmadd_ps(
                        aligned_kernel[k + l + m], data_block, accumulator1);
                }
            }
        }
        _mm256_storeu_ps(out + i, accumulator0);
        _mm256_storeu_ps(out + i + AVX_SIMD_LENGTH, accumulator1);
    }

    // Need to do the last value as a special case
    int i = N_SAMPLES - 1;
    out[i] = 0.0;
    for (int k = 0; k < KERNEL_LENGTH; k++)
    {
        out[i] += h[k] * padded[i + k];
    }

    free(padded);
}

#endif

/*
Calculate the different coefficients depeding on the antenna's direction and size
*/
void directional_antenna_delay_coefficients(double azimuth,   // Horizontal
                                            double elevation, // Vertical
                                            int columns,
                                            int rows,
                                            float distance, // Distance between elements
                                            float fs,
                                            float propagation_speed,
                                            float **coefficients)

{
    // Convert listen direction to radians
    double theta = azimuth * -(double)PI / 180.0;
    double phi = elevation * -(double)PI / 180.0;

    float x_factor = (float)(sin(theta) * cos(phi));
    float y_factor = (float)(cos(theta) * sin(phi));

    // Allocate antenna array
    float *antenna_array = malloc((columns * rows) * sizeof(float));

    int element_index = 0;

    float smallest = 0.0;

    // Create antenna in space with middle in origo (0, 0)
    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < columns; col++)
        {
            float half = distance / 2.0;

            // Assign middle of array to origo
            float tmp_col = (float)col * distance - (float)columns * half + half;
            float tmp_row = (float)row * distance - (float)rows    * half + half;

            float tmp_delay = tmp_col * x_factor + tmp_row * y_factor;

            // Update so there is always one element furthest from world at 0 i.e all other delays are greater
            if (tmp_delay < smallest)
            {
                smallest = tmp_delay;
            }

            antenna_array[element_index] = tmp_delay;

            element_index += 1;
        }
    }

    // Create a delay map
    for (int i = 0; i < rows * columns; i++)
    {   
        // Make the furthest element from source direction have no delay
        if (smallest < 0.0)
        {
            antenna_array[i] -= smallest;
        }

        antenna_array[i] *= fs / propagation_speed;
    }

    double epsilon = 1e-9;  // Small number to avoid dividing by 0

    // Give each element it's own set of coefficients
    for (int element = 0; element < rows * columns; element++)
    {
        double sum = 0.0;

        // This is the crucial math
        double tau = 0.5 - (double)antenna_array[element] + epsilon;

        for (int i = 0; i < N_TAPS; i++)
        {
            // Fractional delay with support to delay entire frames up to OFFSET
            double h_i_d = (double)i - ((double)N_TAPS - 1.0) / 2.0 - tau;
            // Compute the sinc value: sin(xπ)/xπ
            h_i_d = sin(h_i_d * PI) / (h_i_d * PI);

            // To get np.arange(1-M, M, 2)
            double n = (double)(i * 2 - N_TAPS + 1);

            // Multiply sinc value by Blackman-window (https://numpy.org/doc/stable/reference/generated/numpy.blackman.html)
            double black_manning = 0.42 + 0.5 * cos(PI * n / ((double)(N_TAPS - 1)) + epsilon) + 0.08 * cos(2.0 * PI * n / ((double)(N_TAPS - 1) + epsilon));

            h_i_d *= black_manning;

            sum += h_i_d;

            coefficients[element][i] = (float)h_i_d;
        }

        for (int i = 0; i < N_TAPS; i++)
        {
            // Normalize to get unity gain.
            coefficients[element][i] /= (float)sum;
        }
    }

    free(antenna_array);
}