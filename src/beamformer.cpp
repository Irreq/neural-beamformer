#include <iostream>
#include "beamformer.h"
#include "receiver.h"
#include "ring_buffer.h"
#include "config.h"
#include "antenna.h"
#include "delay.h"
#include "hmi.h"
// #include <Eigen/Dense>

using namespace std;
// using namespace Eigen;

bool running = true;

#include <cmath>

#include <unistd.h>
#include <sys/socket.h>
// #include <netinet/in.h>
#include <arpa/inet.h>

class Beamformer {
public:
    Beamformer();
    Beamformer(Beamformer &&) = default;
    Beamformer(const Beamformer &) = default;
    Beamformer &operator=(Beamformer &&) = default;
    Beamformer &operator=(const Beamformer &) = default;
    ~Beamformer();

    void _loop()
    {

        for (int i = 0; i < BUFFER_LENGTH + 100; i++) {
            for (int k = 0; k < N_SENSORS; k++)
            {
                frame[k] = (float)i;
            }

            write_buffer_single(rb, &frame[0]);
        }

        float frames[N_SENSORS][BUFFER_LENGTH];

        read_buffer_all(rb, &frames[0]);

        for (int i = 0; i < BUFFER_LENGTH; i++)
        {
            std::cout << frames[0][i] << " ";
        }

        std::cout << frames << std::endl;

        

        
    }

    void loop() 
    {

        float image[Y][X];
        while (running)
        {
            for (int i = 0; i < N_SAMPLES*2; i++)
            {
                receive(rb);
            }
            
            int i = 0;

            float mean = 0.0;
            float heatmap_data[X * Y];
            for (int y = 0; y < Y; y++)
            {
                for (int x = 0; x < X; x++)
                {
                    float out[N_SAMPLES] = {0.0};
                    for (int s = 0; s < N_SENSORS; s++)
                    {
    
                        float del = flat_delays[i * N_SENSORS + s];
                        naive_delay(rb, &out[0], del, s);
                    }

                    int n = N_SAMPLES;

                    float power = 0.f;
                    for (int p = 0; p < n; p++)
                    {

                        float val = out[p] / (float)N_SENSORS;

                        power += powf(val, 2);
                        out[p] = 0.0;
                    }

                    power /= (float)N_SAMPLES;

                    mean += power;

                    image[y][x] = power;
                    heatmap_data[i] = power;

                    i++;
                }
            }

            transmit(&heatmap_data[0], X * Y);

        }
    }

    

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

            // printf("\r                                                                              \r");
            printf("\n");
            int n = N_SAMPLES;

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

std::vector<Eigen::VectorXf> compute_scanning_window(Eigen::MatrixXf antenna, float fov, int resolution_x, int resolution_y)
    {
        double x, y;

        // Eigen::MatrixXf antenna = create_antenna(Vector3f(0, 0, 0), COLUMNS, ROWS, DISTANCE);
        Eigen::MatrixXf tmp;
        std::vector<Eigen::VectorXf> delays;

        for (int yi = 0; yi < resolution_y; yi++)
        {
            y = atan(2.0*(double)yi / (resolution_y - 1) - 1);
            for (int xi = 0; xi < resolution_x; xi++)
            {
                x = atan(2.0*(double)xi / (resolution_x - 1) - 1);
                // cout << "(" << x * 180.0/M_PI << "," << y * 180.0/M_PI << ")" << endl;

                tmp = steer(antenna, x * 180.0/M_PI, y * 180.0/M_PI);

                Eigen::VectorXf tmp_delays = compute_delays(tmp);

                delays.push_back(compute_delays(tmp));
            }
        }

        return delays;
    }

void compute_scanning_window(float *flat_delays, Eigen::MatrixXf antenna, float fov, int resolution_x, int resolution_y)
    {
        double x, y;

        // Eigen::MatrixXf antenna = create_antenna(Vector3f(0, 0, 0), COLUMNS, ROWS, DISTANCE);
        Eigen::MatrixXf tmp;
        std::vector<Eigen::VectorXf> delays;

        int k = 0;

        for (int yi = 0; yi < resolution_y; yi++)
        {
            y = atan(2.0*(double)yi / (resolution_y - 1) - 1);
            for (int xi = 0; xi < resolution_x; xi++)
            {
                x = atan(2.0*(double)xi / (resolution_x - 1) - 1);
                // cout << "(" << x * 180.0/M_PI << "," << y * 180.0/M_PI << ")" << endl;

                tmp = steer(antenna, x * 180.0/M_PI, y * 180.0/M_PI);

                Eigen::VectorXf tmp_delays = compute_delays(tmp);
                int i = 0;
                for (float del: tmp_delays)
                {
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
    compute_scanning_window(&this->flat_delays[0], antenna, 150.0, this->X, this->Y);
    
}

Beamformer::~Beamformer() {
    
    destroy_ring_buffer(rb);
    stop_receiving();
    stop_hmi();
    
}



// Handle Ctrl-C
void sig_handler(int sig)
{
    running = false;
    
}

#include <csignal>

#if 1

int main (int argc, char *argv[]) {
    Beamformer bf;

    signal(SIGINT, sig_handler);
    signal(SIGKILL, sig_handler);

    bf.loop();

    return 0;
}

#else

void compute_scanning_window(float fov, int resolution_x, int resolution_y)
    {
        double x, y;

        Eigen::MatrixXf antenna = create_antenna(Vector3f(0, 0, 0), COLUMNS, ROWS, DISTANCE);
        Eigen::MatrixXf tmp;
        std::vector<Eigen::VectorXf> delays;

        for (int yi = 0; yi < resolution_y; yi++)
        {
            y = atan(2.0*(double)yi / (resolution_y - 1) - 1);
            for (int xi = 0; xi < resolution_x; xi++)
            {
                x = atan(2.0*(double)xi / (resolution_x - 1) - 1);
                cout << "(" << x * 180.0/M_PI << "," << y * 180.0/M_PI << ")" << endl;

                tmp = steer(antenna, x * 180.0/M_PI, y * 180.0/M_PI);
                delays.push_back(compute_delays(tmp));
            }
        }

        for (auto delay : delays) 
        {
            cout << delay << endl;
        }
    }

int main (int argc, char *argv[]) {
  
    compute_scanning_window(90.f, 10, 8);

    return 0;
}

#endif
