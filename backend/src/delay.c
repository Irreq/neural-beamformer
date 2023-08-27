#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "delay.h"
#include "ring_buffer.h"



/**Requires the signal to be longer than N_SAMPLES and that it starts at BUFFER_LENGTH - N_SAMPLES - 1*/
void lerp_delay(float *signal, float *out, float delay)
{
    double _offset;
    float fraction;

    fraction = (float)modf((double)delay, &_offset);

    int offset = (int)_offset;

    printf("%f = %d, %f\n", delay, offset, fraction);

    for (int i = 0; i < N_SAMPLES; i++)
    {
        out[i] += signal[i - offset - 1] + fraction * (signal[i - offset - 1] - signal[i - offset]);
    }

    // for (int i = 0; i < N_SAMPLES - offset - 1; i++)
    // {
    //     out[offset + i + 1] += signal[i] + h * (signal[i + 1] - signal[i]); // Must precalc h = 1 - h
    // }
}

#define SAFETY_CHECK 1

int delay(ring_buffer *rb, float *out, float delay, int sensor_id)
{
    double _offset;
    float fraction;

    fraction = (float)modf((double)delay, &_offset);

    int offset = (int)_offset;

#if SAFETY_CHECK
    if ((offset >= BUFFER_LENGTH - N_SAMPLES) || (offset < 0))
    {
        printf("Out of bounds delay, increase buffer size\n");
        exit(1);
    }
#endif

    int next, current, start;

    float *signal = &rb->data[sensor_id][0];


    // Start index begins from the start of the latest N_SAMPLES - offset and forwards
    start = rb->index + BUFFER_LENGTH - N_SAMPLES - offset;

    printf("Start pos: %d (%d.%f)\n", start, offset, fraction);

    for (int i = 0; i < N_SAMPLES; i++)
    {
        current = start + i;
        next = current + 1;

        current &= (BUFFER_LENGTH - 1);
        next &= (BUFFER_LENGTH - 1);

        // printf("(%d %d) ", current, next);

        out[i] += signal[current] + fraction * (signal[current] - signal[next]);
    }
}


int old_main()
{
    ring_buffer *rb;
    rb = create_ring_buffer();

    float data[N_SAMPLES];

    float out[BUFFER_LENGTH];

    for (int k = 0; k < 8; k++)
    {
        for (int i = 0; i < N_SAMPLES; i++)
        {
            data[i] = (float)(i+k);
        }

        write_buffer_single(rb, &data[0]);
    }

    for (int i = 0; i < N_SAMPLES; i++)
    {
        data[i] = 0.0;
    }

    for (int i = 0; i < BUFFER_LENGTH; i++)
    {
        out[i] = (float)i;
    }


    lerp_delay(&out[BUFFER_LENGTH - N_SAMPLES], &data[0], 1.0);

    for (int i = 0; i < N_SAMPLES; i++)
    {
        printf("(%f %f) ", out[BUFFER_LENGTH - 1 - N_SAMPLES + i], data[i]);
    }

    printf("\n");
}

int main()
{
    ring_buffer *rb;
    rb = create_ring_buffer();

    float data[N_SENSORS][N_SAMPLES];

    int count = 0;

    for (int x = 0; x < 11; x++)
    {

        for (int k = 0; k < N_SENSORS; k++)
        {
            for (int i = 0; i < N_SAMPLES; i++)
            {
                data[k][i] = (float)(i+k+count);
            }
        }

        write_buffer_all(rb, &data[0][0]);

        count += N_SAMPLES;
    }

    float result[N_SAMPLES];

    for (int i = 0; i < N_SAMPLES; i++)
    {
        result[i] = 0.0;
    }

    int index = 0;

    float h = 1.5;

    delay(rb, &result[0], h, index);

    float buf[N_SENSORS][BUFFER_LENGTH];

    read_buffer_all(rb, &buf[0]);

    printf("\n");

    for (int i = 0; i < N_SAMPLES; i++)
    {
        printf("(%f %f)\n", buf[index][BUFFER_LENGTH - N_SAMPLES + i], result[i]);
    }

    printf("\n");
}

// int test_delay()
// {
//     _main();
//     printf("Hello World2!\n");

//     return 0;
// }