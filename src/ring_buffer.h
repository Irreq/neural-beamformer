#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include "config.h"
// typedef struct _ring_buffer
// {
//     int index;
//     float data[BUFFER_LENGTH];
//     int counter;
//     int can_read;
// } ring_buffer;

/**

Remember that BUFFER_LENGTH must be at least MAX_DELAY_FRAMES + 1

where MAX_DELAY_FRAMES is the maximum delay of a the furthest sensor

*/
typedef struct __ring_buffer
{
    int index;
    float data[N_SENSORS][BUFFER_LENGTH];
} ring_buffer;

ring_buffer *create_ring_buffer();

ring_buffer *destroy_ring_buffer(ring_buffer *rb);

void write_buffer(ring_buffer *rb, float *in);

void write_buffer_single(ring_buffer *rb, float *data);

void read_mcpy(ring_buffer *rb, float *out);

void write_buffer_all(ring_buffer *rb, float (*data)[N_SAMPLES]);

void read_buffer_all(ring_buffer *rb, float (*out)[BUFFER_LENGTH]);

#endif