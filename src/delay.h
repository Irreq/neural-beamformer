#ifndef DELAY_H
#define DELAY_H

#include "ring_buffer.h"

int test_delay();
/**Requires the signal to be longer than N_SAMPLES and that it starts at BUFFER_LENGTH - N_SAMPLES - 1*/
void lerp_delay(float *signal, float *out, float delay);

int delay(ring_buffer *rb, float *out, float delay, int sensor_id);

#endif
