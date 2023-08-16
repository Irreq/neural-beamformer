#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>

#include "config.h"
#include "ring_buffer.h"

#ifndef AVX
#define AVX 0
#endif

#if AVX
#include <immintrin.h>
#endif

ring_buffer *rb;

RB *rb2;

int shm_fd;

pthread_t threads[N_THREADS];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barrier;

int stop_processing = 0; // Flag to signal threads to stop processing

typedef struct {
    pthread_t threads[N_THREADS];
    float kernels[N_KERNELS];
    int data_size;
    int current_task;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int stop;
} ThreadPool;

ThreadPool *pool;


void signal_handler(int signum) {
    // Set the stop_processing flag to terminate worker threads gracefully
    stop_processing = 1;
}


void* worker_function(void* arg) {
    float out[BUFFER_LENGTH];
    while (!stop_processing) {
        // Wait for new batch of data
        pthread_barrier_wait(&barrier);

        if (stop_processing) {
            break; // Exit the loop if the flag is set
        }

        read_mcpy(rb, &out[0]);

        while (1)
        {
            // Worker threads process shared data (convolution)
            pthread_mutex_lock(&mutex);

            if (pool->current_task >= N_KERNELS)
            {
                pthread_mutex_unlock(&mutex);
                break;
            }

            pool->current_task++;
            // read_mcpy(rb, &out[0]);

            printf("Worker %ld (%d) processed: ", (long)arg, pool->current_task, out[0]);

            

            // for (int i = 0; i < BUFFER_LENGTH; i++)
            // {
            //     printf("%f ", out[i]);
            // }

            printf("\n");
            

            pthread_mutex_unlock(&mutex);

            usleep(10000);
        }
    }
    return NULL;
}

#if 0

int main() {
    pool = (ThreadPool *)calloc(1, sizeof(ThreadPool));
    // pool->kernels = malloc(sizeof(float) * N_KERNELS);
    pool->data_size = N_KERNELS;
    pool->current_task = 0;
    pool->stop = 0;

    signal(SIGINT, signal_handler);

    shm_fd = shm_open("/shared_memory", O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shm_fd, sizeof(ring_buffer)) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    rb = mmap(NULL, sizeof(ring_buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (rb == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    pthread_barrier_init(&barrier, NULL, N_THREADS + 1);

    // Initialize worker threads
    
    for (long i = 0; i < N_THREADS; ++i) {
        pthread_create(&threads[i], NULL, worker_function, (void*)i);
    }

    float data[N_SAMPLES];

    float count = 0;

    for (int i = 0; i < 10; i++)
    {
        // Generate dummy data (for demonstration)
        for (int i = 0; i < N_SAMPLES; ++i) {
            data[i] = rand() % 100; // Replace with actual data source
            data[i] = count;
            count += 1.0;
        }

        write_buffer(rb, &data[0]);
    }
    // Main loop for the producer
    while (!stop_processing) {
        // Generate dummy data (for demonstration)
        for (int i = 0; i < N_SAMPLES; ++i) {
            // data[i] = rand() % 100; // Replace with actual data source
            data[i] = count;
            count += 1.0;
        }

        pthread_mutex_lock(&mutex);

        write_buffer(rb, &data[0]);

        pool->current_task = 0;

        pthread_mutex_unlock(&mutex);

        
        
        // Signal worker threads to process the data
        pthread_barrier_wait(&barrier);
        sleep(1); // Simulate processing time (adjust as needed)
    }
    pthread_barrier_wait(&barrier);

    printf("Cleaning\n");

    // Cleanup
    for (long i = 0; i < N_THREADS; ++i) {
        pthread_cancel(threads[i]);
    }

    // printf("Could cancel all threads\n");
    pthread_barrier_destroy(&barrier);
    // printf("barrier\n");
    munmap(rb, sizeof(ring_buffer));
    // printf("nunmap\n");
    close(shm_fd);
    // printf("close\n");
    shm_unlink("/shared_memory");

    free(pool);

    exit(0);

    return 0;
}

#else
#include <string.h>
int main()
{
#if AVX
    printf("Using AVX\n");

    // Align the RB structure's data array to 32 bytes for AVX
    rb2 = (RB *)_mm_malloc(sizeof(RB), 32);
    if (!rb2) {
        // Handle allocation failure
        return 1;
    }
    float arr[N_SENSORS][N_SAMPLES] __attribute__((aligned(32)));
#else
    rb2 = (RB *)calloc(1, sizeof(RB));
    float arr[N_SENSORS][N_SAMPLES];
#endif
    float out[N_SENSORS][BUFFER_LENGTH];

    // int i = 0;

    for (int s = 0; s < N; s++)
    {
        for (int k = 0; k < N_SENSORS; k++)
        {
            for (int i = 0; i < N_SAMPLES; i++)
            {
                arr[k][i] = (float)(s * N_SAMPLES + 10 * k + i);
            }
        }

        write_buffer_all_avx(rb2, &arr[0]);
        
    }

#if 1

    clock_t tic, toc;
    double duration;

    int n = 100000;

    tic = clock();

   

    for (int i = 0; i < n; i++)
    {

        // write_buffer_all_avx(rb2, &arr[0]);
        read_buffer_all_avx(rb2, &out[0]);
    }

    toc = clock();

    duration = (double)(toc - tic) / CLOCKS_PER_SEC;

    printf("AVX Elapsed: %f seconds\n", duration);

    tic = clock();

    for (int i = 0; i < n; i++)
    {

        // write_buffer_all(rb2, &arr[0]);
        read_buffer_all(rb2, &out[0]);
    }

    toc = clock();

    duration = (double)(toc - tic) / CLOCKS_PER_SEC;

    printf("Naive Elapsed: %f seconds\n", duration);
#else
    read_buffer_all_avx(rb2, &out[0]);

    for (int k = 0; k < N_SENSORS; k++)
    {
        for (int i = 0; i < BUFFER_LENGTH; i++)
        {
            printf("%d ", (int)out[k][i]);
        }

        printf("\n");
    }
#endif

#if AVX
    // Free the aligned memory when done
    _mm_free(rb);
#else
    free(rb2);
#endif
}

#endif
