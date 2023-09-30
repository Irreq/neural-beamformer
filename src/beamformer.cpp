#include <iostream>
#include "beamformer.h"
#include "receiver.h"
#include "ring_buffer.h"
#include "config.h"
#include "antenna.h"
#include "delay.h"
// #include <Eigen/Dense>

using namespace std;
// using namespace Eigen;

bool running = true;

#include <cmath>

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

            printf("%f ", power);
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
    float frame[N_SENSORS];
    float latest[N_SENSORS][BUFFER_LENGTH];
    ring_buffer *rb;
    Eigen::MatrixXf antenna;
};

Beamformer::Beamformer() {
    rb = create_ring_buffer();
    init_receiver();
    antenna = create_antenna(Vector3f(0, 0, 0), COLUMNS, ROWS, DISTANCE);
    antenna = steer(antenna, 30.0, 10.0);
}

Beamformer::~Beamformer() {
    destroy_ring_buffer(rb);
    stop_receiving();
}



// Handle Ctrl-C
void sig_handler(int sig)
{
    running = false;
    
}

#include <csignal>

int main (int argc, char *argv[]) {
    Beamformer bf;

    signal(SIGINT, sig_handler);
    signal(SIGKILL, sig_handler);

    bf.loop();

    return 0;
}
