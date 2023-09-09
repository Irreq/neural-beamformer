#include <iostream>
#include "beamformer.h"
#include "ring_buffer.h"
#include "config.h"

class Beamformer {
public:
    Beamformer();
    Beamformer(Beamformer &&) = default;
    Beamformer(const Beamformer &) = default;
    Beamformer &operator=(Beamformer &&) = default;
    Beamformer &operator=(const Beamformer &) = default;
    ~Beamformer();

    void loop()
    {

        for (int i = 0; i < 50; i++) {
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

private:
    float frame[N_SENSORS];
    ring_buffer *rb;
};

Beamformer::Beamformer() {
    rb = create_ring_buffer();
}

Beamformer::~Beamformer() {
    destroy_ring_buffer(rb);
}


int main (int argc, char *argv[]) {
    Beamformer bf;

    bf.loop();
    return 0;
}
