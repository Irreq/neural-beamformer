#include <stdlib.h>
#include <string.h> // memcpy
#include <stdint.h>
#include <stdio.h>

#include <time.h>
#include <math.h>

#include "ring_buffer.h"
#include "config.h"
#include "utils.h"


#define UNROLLED 0


/*
Create a ring buffer
*/
ring_buffer *create_ring_buffer(){
    ring_buffer *rb = (ring_buffer *)calloc(1, sizeof(ring_buffer));
    rb->index = 0;
    return rb;
}

/*
Destroy a ring buffer
*/
ring_buffer *destroy_ring_buffer(ring_buffer *rb){
    free(rb);
    rb = NULL;
    return rb;
}

#if UNROLLED

inline void write_buffer(ring_buffer *rb, float *in)
{
    int index = rb->index;
    for (int i = 0; i < N_SAMPLES; i += 8)
    {
        int idx1 = (index + i + 0) & (BUFFER_LENGTH - 1);
        int idx2 = (index + i + 1) & (BUFFER_LENGTH - 1);
        int idx3 = (index + i + 2) & (BUFFER_LENGTH - 1);
        int idx4 = (index + i + 3) & (BUFFER_LENGTH - 1);
        int idx5 = (index + i + 4) & (BUFFER_LENGTH - 1);
        int idx6 = (index + i + 5) & (BUFFER_LENGTH - 1);
        int idx7 = (index + i + 6) & (BUFFER_LENGTH - 1);
        int idx8 = (index + i + 7) & (BUFFER_LENGTH - 1);
        
        rb->data[idx1] = in[i + 0];
        rb->data[idx2] = in[i + 1];
        rb->data[idx3] = in[i + 2];
        rb->data[idx4] = in[i + 3];
        rb->data[idx5] = in[i + 4];
        rb->data[idx6] = in[i + 5];
        rb->data[idx7] = in[i + 6];
        rb->data[idx8] = in[i + 7];
    }

    // Sync current index
    rb->index = (index + N_SAMPLES) & (BUFFER_LENGTH - 1);
}

#elif 1

/** Fast memcpy write to a ringbuffer, requires N_SAMPLES % 2 AND BUFFER_LENGTH % 2
*/
inline void write_buffer(ring_buffer *rb, float *in)
{
    memcpy((void *)&rb->data[rb->index], in, sizeof(float) * N_SAMPLES);
    rb->index = (rb->index + N_SAMPLES) & (BUFFER_LENGTH - 1);
}



#else
/*
Write data from an address `in` to a ring buffer you can specify offset
but most of the times, it will probably just be 0
*/
inline void write_buffer(ring_buffer *rb, float *in)
{
    int buffer_length = BUFFER_LENGTH - 1;
    int previous_item = rb->index;

    int idx;
    for (int i = 0; i < N_SAMPLES; ++i)
    {
        
        idx = (i + previous_item) & buffer_length; // Wrap around
        rb->data[idx] = in[i];
    }

    // Sync current index
    rb->index += N_SAMPLES;
    rb->index &= BUFFER_LENGTH - 1;
}

#endif

// inline void read_mcpy(ring_buffer *rb, float *out)
// {
//     int first_partition = BUFFER_LENGTH - rb->index;

//     float *data_ptr = &rb->data[0];

//     memcpy(out, (void *)(data_ptr + rb->index), sizeof(float) * first_partition);
//     memcpy(out + first_partition, (void *)(data_ptr), sizeof(float) * rb->index);
// }

void write_buffer_single(ring_buffer *rb, float *data)
{
    for (int i = 0; i < N_SENSORS; i++)
    {
        rb->data[i][rb->index] = data[i];
    }



    rb->index = (rb->index + 1) & (BUFFER_LENGTH - 1);
}

#include <unistd.h>
void write_buffer_single_int32(ring_buffer *rb, int32_t *data)
{
    int inverted = 0;
    for (int i = 0; i < N_SENSORS; i++)
    {
        if (i % COLUMNS == 0)
        {
            inverted = !inverted;
        }

        if (inverted) {
            rb->data[i][rb->index] = normalize_int32(data[i]);
        } else {
            int k = COLUMNS * (1 + i / COLUMNS) - i % COLUMNS;
            rb->data[i][rb->index] = normalize_int32(data[k]);
        }
    }

    rb->index = (rb->index + 1) & (BUFFER_LENGTH - 1);
}


#define AVX 0
#if AVX

#include <immintrin.h>

inline void write_buffer_all(ring_buffer *rb, float (*data)[N_SAMPLES]) {
    for (int i = 0; i < N_SENSORS; i++) {
        for (int k = 0; k < N_SAMPLES; k+=8*2)
        {
            _mm256_storeu_ps(&rb->data[i][rb->index + k], _mm256_loadu_ps(&data[i][k]));
            _mm256_storeu_ps(&rb->data[i][rb->index + (k + 8)], _mm256_loadu_ps(&data[i][k+8]));
        }
        
    }

    rb->index = (rb->index + N_SAMPLES) & (BUFFER_LENGTH - 1);
}

void read_buffer_all(ring_buffer *rb, float (*out)[BUFFER_LENGTH]) {
    int first_partition = BUFFER_LENGTH - rb->index;

    for (int i = 0; i < N_SENSORS; i++) {
        int j = 0;

        for (; j < first_partition; j += 8) {
            __m256 source = _mm256_loadu_ps(&rb->data[i][rb->index + j]);
            _mm256_storeu_ps(&out[i][j], source);
        }

        // Copy the elements from the beginning of the buffer
        for (int k = 0; k < rb->index; k += 8) {
            __m256 source = _mm256_loadu_ps(&rb->data[i][k]);
            _mm256_storeu_ps(&out[i][j + k], source);
        }
    }
}

#else

void write_buffer_all(ring_buffer *rb, float (*data)[N_SAMPLES])
{
    for (int i = 0; i < N_SENSORS; i++)
    {
        memcpy((void *)&rb->data[i][rb->index], &data[i][0], sizeof(float) * N_SAMPLES);
    }

    rb->index = (rb->index + N_SAMPLES) & (BUFFER_LENGTH - 1);
}

void read_buffer_all(ring_buffer *rb, float (*out)[BUFFER_LENGTH])
{
    int first_partition = BUFFER_LENGTH - rb->index;

    for (int i = 0; i < N_SENSORS; i++)
    {
        memcpy(&out[i][0], (void *)&rb->data[i][rb->index], sizeof(float) * first_partition);
        memcpy(&out[i][0] + first_partition, (void *)&rb->data[i][0], sizeof(float) * rb->index);
    }

}

#endif
