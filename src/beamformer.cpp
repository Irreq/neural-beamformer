#include "beamformer.h"
#include "antenna.h"
#include "config.h"
#include "delay.h"
#include "hmi.h"
#include "receiver.h"
#include "ring_buffer.h"
#include <iostream>
// #include <Eigen/Dense>

using namespace std;
// using namespace Eigen;

bool running = true;

#include <cmath>

#include <sys/socket.h>
#include <unistd.h>
// #include <netinet/in.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
int shm_fd;

// ring_buffer *rb;

pthread_t threads[N_THREADS];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barrier;

pthread_barrier_t barrier2;

int stop_processing = 0; // Flag to signal threads to stop processing

#define _X 20
#define _Y _X

typedef struct {
  pthread_t threads[N_THREADS];
  float kernels[N_KERNELS];
  float flat_delays[_X * _Y * N_SENSORS];
  int limit;
  int active_sensors;
  int data_size;
  int current_task;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  int stop;
  float heatmap_data[_X * _Y];
  float result[N_SENSORS];
  float data[N_SENSORS][BUFFER_LENGTH];
  ring_buffer *rb;
} ThreadPool;

ThreadPool *pool;

float compute(int t_id, int task, float *flat_delays) {
  float out[N_SAMPLES] = {0.0};
  for (int s = 0; s < N_SENSORS; s++) {

    float del = flat_delays[s];
    naive_delay(pool->rb, &out[0], del, s);
  }

  int n = pool->active_sensors;

  float power = 0.f;
  for (int p = 0; p < n; p++) {

    float val = out[p] / (float)n;

    power += powf(val, 2);
  }

  return power / (float)N_SAMPLES;
}

/**
 * Worker functionss for threaded operation
 */
void *worker_function(void *arg) {
  int task;
  while (!stop_processing) {
    // Wait for new batch of data
    pthread_barrier_wait(&barrier);

    if (stop_processing) {
      break; // Exit the loop if the flag is set
    }

    while (1) {
      pthread_mutex_lock(&mutex);
      if (pool->current_task >= pool->limit) {
        pthread_mutex_unlock(&mutex);
        break;
      }

      task = pool->current_task++;

      pthread_mutex_unlock(&mutex);

      // printf("%d ", task);

      pool->result[task] =
          compute(*(int *)arg, task, &pool->flat_delays[task * N_SENSORS]);
    }
  }
  return NULL;
}

class Beamformer {
public:
  Beamformer();
  Beamformer(Beamformer &&) = default;
  Beamformer(const Beamformer &) = default;
  Beamformer &operator=(Beamformer &&) = default;
  Beamformer &operator=(const Beamformer &) = default;
  ~Beamformer();

  void _loop() {

    for (int i = 0; i < BUFFER_LENGTH + 100; i++) {
      for (int k = 0; k < N_SENSORS; k++) {
        frame[k] = (float)i;
      }

      write_buffer_single(rb, &frame[0]);
    }

    float frames[N_SENSORS][BUFFER_LENGTH];

    read_buffer_all(rb, &frames[0]);

    for (int i = 0; i < BUFFER_LENGTH; i++) {
      std::cout << frames[0][i] << " ";
    }

    std::cout << frames << std::endl;
  }

  void loop() {

    // float image[Y][X];
    while (running) {
      for (int i = 0; i < N_SAMPLES * 2; i++) {
        receive(rb);
      }

      int i = 0;

      float mean = 0.0;
      float heatmap_data[X * Y];
      for (int y = 0; y < Y; y++) {
        for (int x = 0; x < X; x++) {
          float out[N_SAMPLES] = {0.0};
          for (int s = 0; s < N_SENSORS; s++) {

            float del = flat_delays[i * N_SENSORS + s];
            naive_delay(rb, &out[0], del, s);
          }

          int n = N_SAMPLES;

          float power = 0.f;
          for (int p = 0; p < n; p++) {

            float val = out[p] / (float)N_SENSORS;

            power += powf(val, 2);
            out[p] = 0.0;
          }

          power /= (float)N_SAMPLES;

          mean += power;

          // image[y][x] = power;
          heatmap_data[i] = power;

          i++;
        }
      }

      transmit(&heatmap_data[0], X * Y);
    }
  }

  /*
  void __loop()
  {
      while (running)
      {
          receive(rb);

          read_buffer_all(rb, &latest[0]);

          Eigen::VectorXf delays = compute_delays(antenna);

          float result[N_SENSORS][N_SAMPLES];

          float out[N_SAMPLES] = {0.0};

          for (int i = 0; i < N_SENSORS; i++)
          {
              naive_delay(rb, &out[0], delays.coeff(i), i);
          }

          // printf("\r \r"); printf("\n"); int n = N_SAMPLES;

          float power = 0.f;
          for (int i = 0; i < n; i++)
          {

              float val = out[i] / (float)N_SENSORS;

              power += powf(val, 2);

              // printf("%f ", out[i]);
          }

          power /= (float)N_SAMPLES;

          printf("%.10f ", power);
      }
  }
  */

  /*
  void mimo(float (&image)[N_SENSORS])
  {
      for (int i = 0; i < N_SENSORS; i++)
      {
          for (int k = 0; k < N_SAMPLES; k++)
          {
              ;
          }
      }
  }

  */
private:
  static const int X = 40;
  static const int Y = 40;
  float frame[N_SENSORS];
  float latest[N_SENSORS][BUFFER_LENGTH];
  float flat_delays[X * Y * N_SENSORS];
  ring_buffer *rb;
  Eigen::MatrixXf antenna;
  std::vector<Eigen::VectorXf> delays;
};

// TODO fov is not used, must be multiplied with x amd y, otherwise we will get
// 45degree every time
std::vector<Eigen::VectorXf> compute_scanning_window(Eigen::MatrixXf antenna,
                                                     float fov,
                                                     int resolution_x,
                                                     int resolution_y) {
  double x, y;

  // Eigen::MatrixXf antenna = create_antenna(Vector3f(0, 0, 0), COLUMNS, ROWS,
  // DISTANCE);
  Eigen::MatrixXf tmp;
  std::vector<Eigen::VectorXf> delays;

  for (int yi = 0; yi < resolution_y; yi++) {
    y = atan(2.0 * (double)yi / (resolution_y - 1) - 1);
    for (int xi = 0; xi < resolution_x; xi++) {
      x = atan(2.0 * (double)xi / (resolution_x - 1) - 1);
      // cout << "(" << x * 180.0/M_PI << "," << y * 180.0/M_PI << ")" << endl;

      tmp = steer(antenna, x * 180.0 / M_PI, y * 180.0 / M_PI);

      Eigen::VectorXf tmp_delays = compute_delays(tmp);

      delays.push_back(compute_delays(tmp));
    }
  }

  return delays;
}

/**
 * Calculates the requred angles for looking in at a plane in front of the array
 */
void compute_scanning_window(float *flat_delays, Eigen::MatrixXf antenna,
                             float fov, int resolution_x, int resolution_y) {
  double x, y;

  // Eigen::MatrixXf antenna = create_antenna(Vector3f(0, 0, 0), COLUMNS, ROWS,
  // DISTANCE);
  Eigen::MatrixXf tmp;
  std::vector<Eigen::VectorXf> delays;

  int k = 0;

  for (int yi = 0; yi < resolution_y; yi++) {
    y = atan(2.0 * (double)yi / (resolution_y - 1) - 1);
    for (int xi = 0; xi < resolution_x; xi++) {
      x = atan(2.0 * (double)xi / (resolution_x - 1) - 1);
      // cout << "(" << x * 180.0/M_PI << "," << y * 180.0/M_PI << ")" << endl;

      tmp = steer(antenna, x * 180.0 / M_PI, y * 180.0 / M_PI);

      Eigen::VectorXf tmp_delays = compute_delays(tmp);
      int i = 0;
      for (float del : tmp_delays) {
        flat_delays[k * N_SENSORS + i] = del;
        // flat_delays[yi * COLUMNS * N_SENSORS + xi * N_SENSORS + i] = del;
        i++;
      }

      k++;
      // delays.push_back(compute_delays(tmp));
    }
  }

  // return delays;
}

Beamformer::Beamformer() {
  start_hmi();
  rb = create_ring_buffer();
  init_receiver();
  antenna = create_antenna(Vector3f(0, 0, 0), COLUMNS, ROWS, DISTANCE);
  antenna = steer(antenna, 0.0, 0.0);

  delays = compute_scanning_window(antenna, FOV, this->X, this->Y);
  compute_scanning_window(&this->flat_delays[0], antenna, 150.0, this->X,
                          this->Y);
}

Beamformer::~Beamformer() {

  destroy_ring_buffer(rb);
  stop_receiving();
  stop_hmi();
}

#include <csignal>

#if 1

void sig_handler(int sig) {
  // Set the stop_processing flag to terminate worker threads gracefully
  stop_processing = 1;
}

int main() {
  pool = (ThreadPool *)calloc(1, sizeof(ThreadPool));
  pool->data_size = N_KERNELS;
  pool->current_task = 0;
  pool->stop = 0;
  pool->active_sensors = N_SENSORS;
  pool->limit = _X * _Y;
  // pool->heatmap_data = {0};

  signal(SIGINT, sig_handler);

  start_hmi();

  init_receiver();

  Eigen::MatrixXf antenna;
  std::vector<Eigen::VectorXf> delays;
  antenna = create_antenna(Vector3f(0, 0, 0), COLUMNS, ROWS, DISTANCE);
  antenna = steer(antenna, 0.0, 0.0);

  delays = compute_scanning_window(antenna, FOV, _X, _Y);
  compute_scanning_window(&pool->flat_delays[0], antenna, 150.0, _X, _Y);

  shm_fd = shm_open("/shared_memory", O_CREAT | O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open");
    exit(EXIT_FAILURE);
  }

  if (ftruncate(shm_fd, sizeof(ring_buffer)) == -1) {
    perror("ftruncate");
    exit(EXIT_FAILURE);
  }

  pool->rb = (ring_buffer *)mmap(NULL, sizeof(ring_buffer),
                                 PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (pool->rb == MAP_FAILED) {
    perror("mmap");
    exit(EXIT_FAILURE);
  }

  pthread_barrier_init(&barrier, NULL, N_THREADS + 1);

  // Initialize worker threads
  for (long i = 0; i < N_THREADS; ++i) {
    pthread_create(&threads[i], NULL, worker_function, (void *)i);
  }

  // pthread_create(&threads[N_THREADS], NULL, waiting_thread, (void*)0);

  float data[N_SENSORS][N_SAMPLES];

  float count = 0;

  // for (int s = 0; s < N_SENSORS; s++)
  // {
  //     // Generate dummy data (for demonstration)
  //     for (int i = 0; i < N_SAMPLES; ++i) {
  //         data[s][i] = rand() % 100; // Replace with actual data source
  //         data[s][i] = (float)(count * N_SAMPLES + s * N_SAMPLES + i);
  //         count += 1.0;
  //     }

  //     write_buffer_all(rb, &data[0]);
  // }

  for (int n = 0; n < N_FRAMES; n++) {
    for (int i = 0; i < N_SAMPLES; i++) {
      receive(pool->rb); // Fill buffer
    }
  }

  // Main loop for the producer
  while (!stop_processing) {

    for (int i = 0; i < N_SAMPLES; i++) {
      receive(pool->rb); // Fill buffer
    }

    pthread_mutex_lock(&mutex);

    // write_buffer_all(rb, &data[0][0]);
    // // write_buffer_single(rb, &data[0][0]);

    // read_buffer_all(rb, &pool->data[0]);

    pool->current_task = 0;

    // printf("New data: ");

    // for (int i = 0; i < pool->limit; i++)
    // {
    //     printf("%f ", pool->result[i]);
    // }

    transmit(&pool->result[0], _X * _Y);

    // printf("\n");

    pthread_mutex_unlock(&mutex);

    // Signal worker threads to process the data
    pthread_barrier_wait(&barrier);

    //  usleep(1000);

    // usleep(100000); // Simulate processing time (adjust as needed)
  }
  // pthread_barrier_wait(&barrier2);
  pthread_barrier_wait(&barrier);

  printf("Cleaning\n");

  // Cleanup
  for (long i = 0; i < N_THREADS; ++i) {
    pthread_cancel(threads[i]);
  }

  // printf("Could cancel all threads\n");
  pthread_barrier_destroy(&barrier);
  // pthread_barrier_destroy(&barrier2);
  // printf("barrier\n");
  munmap(pool->rb, sizeof(ring_buffer));
  // printf("nunmap\n");
  close(shm_fd);
  // printf("close\n");
  shm_unlink("/shared_memory");

  free(pool);

  stop_receiving();
  stop_hmi();

  exit(0);

  return 0;
}

#elif 1

// Handle Ctrl-C
void sig_handler(int sig) { running = false; }

int main() {
  Beamformer bf;

  signal(SIGINT, sig_handler);
  signal(SIGKILL, sig_handler);

  bf.loop();

  return 0;
}

#else

void compute_scanning_window(float fov, int resolution_x, int resolution_y) {
  double x, y;

  Eigen::MatrixXf antenna =
      create_antenna(Vector3f(0, 0, 0), COLUMNS, ROWS, DISTANCE);
  Eigen::MatrixXf tmp;
  std::vector<Eigen::VectorXf> delays;

  for (int yi = 0; yi < resolution_y; yi++) {
    y = atan(2.0 * (double)yi / (resolution_y - 1) - 1);
    for (int xi = 0; xi < resolution_x; xi++) {
      x = atan(2.0 * (double)xi / (resolution_x - 1) - 1);
      cout << "(" << x * 180.0 / M_PI << "," << y * 180.0 / M_PI << ")" << endl;

      tmp = steer(antenna, x * 180.0 / M_PI, y * 180.0 / M_PI);
      delays.push_back(compute_delays(tmp));
    }
  }

  for (auto delay : delays) {
    cout << delay << endl;
  }
}

int main(int argc, char *argv[]) {

  compute_scanning_window(90.f, 10, 8);

  return 0;
}

#endif
