#include <stdio.h>

// For the CUDA runtime routines (prefixed with "cuda_")
#include <cuda_runtime.h>

#include <helper_cuda.h>
#include "../config.h"


__global__ void miso(const float *in, const float *h, float *out)
{
    int i = blockDim.x * blockIdx.x + threadIdx.x;

    int offset;

    if (i < N_SAMPLES - 1)
    {
        out[i] = 0.f;
        for (int k = 0; k < N_SENSORS; k++)
        {
            offset = k * N_SAMPLES + i;
            out[i] += in[offset] + h[k] * (in[offset] - in[offset + 1]);
        }
    }
}


void main()
{
    size_t size = N_SAMPLES * N_SENSORS * sizeof(float);

    float *h_signal = (float *)malloc(size);

    float *h_delay = (float *)malloc(N_SENSORS * sizeof(float));

    float *h_result = (float *)malloc(N_SAMPLES * sizeof(float));

    if (h_signal == NULL || h_delay == NULL || h_result == NULL)
    {
        fprintf(stderr, "Failed to allocate host vectors!\n");
        exit(EXIT_FAILURE);
    }

    for (int k = 0; k < N_SENSORS; k++)
    {
        h_delay[k] = 0.5f; 
        for (int i = 0; i < N_SAMPLES; i++)
        {
            result[i] = 0.f;
            h_signal[k * N_SAMPLES + i] = (float)i;
        }
    }

    printf("All worked well!\n");


    free(h_signal);
    free(h_delay);
    free(h_result);
}
