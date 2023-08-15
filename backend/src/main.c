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

ring_buffer *rb;

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
            // Perform convolution on shared_data
            // For demonstration purposes, let's just print the first element
            // printf("Worker %ld processed: %d\n", (long)arg, shared_data[0]);
            read_mcpy(rb, &out[0]);

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
