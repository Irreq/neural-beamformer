#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include "config.h"
typedef struct _ring_buffer
{
    int index;
    float data[BUFFER_LENGTH];
    int counter;
    int can_read;
} ring_buffer;
#include <immintrin.h>
typedef struct __ring_buffer
{
    int index;
    float data[N_SENSORS][BUFFER_LENGTH];
} RB;

void write_buffer(ring_buffer *rb, float *in);

void read_mcpy(ring_buffer *rb, float *out);

void write_buffer_all(RB *rb, float (*data)[N_SAMPLES]);

void read_buffer_all(RB *rb, float (*out)[BUFFER_LENGTH]);

void read_buffer_all_avx(RB *rb, float (*out)[BUFFER_LENGTH]);


void write_buffer_all_avx(RB *rb, float (*data)[N_SAMPLES]);

#endif