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

void write_buffer(ring_buffer *rb, float *in);

void read_mcpy(ring_buffer *rb, float *out);



#endif