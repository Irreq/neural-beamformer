

import numpy as np

import python.antenna as ant

from config import *

# Time axis
#t = np.linspace(0, (N_SAMPLES - 1) / SAMPLE_RATE, N_SAMPLES)

# ELEMENT_DISTANCE = 0.01
# PROPAGATION_SPEED = 340
# ANGLE_LIMIT = 90

# MIN_FREQUENCY = 1800
# MAX_FREQUENCY = 2000

# MIN_SIGNAL_AMPLITUDE = 0.5
# MAX_SIGNAL_AMPLITUDE = 1.0

# MAX_NOISE_AMPLITUDE = 0.4
# MIN_NOISE_AMPLITUDE = 0.0

# CORRUPTION_NOISE_AMPLITUDE = 0.01


def generate_signal(frequency: float, amplitude: float, phase: float):
    # Generate the sine wave
    return amplitude * np.sin(2 * np.pi * frequency * t + phase)

def sine_wave_generator(amplitude, frequency, sample_rate, phase = 0.0):
    """
    Generate a continuous sine wave.

    Args:
        amplitude (float): The amplitude of the sine wave.
        frequency (float): The frequency of the sine wave in Hertz (Hz).
        sample_rate (int): The number of samples per second.

    Yields:
        float: The next sample value in the sine wave.
    """
    t = 0  # Initialize time
    while True:
        # Calculate the next sample value using the sine function
        sample = amplitude * np.sin(2 * np.pi * frequency * t + phase)
        yield sample
        t += 1 / sample_rate

def sine_wave_samples_generator(amplitude, frequency, sample_rate, n_samples = N_SAMPLES, phase = 0.0):
    """
    Generate a continuous sine wave.

    Args:
        amplitude (float): The amplitude of the sine wave.
        frequency (float): The frequency of the sine wave in Hertz (Hz).
        sample_rate (int): The number of samples per second.

    Yields:
        float: The next sample value in the sine wave.
    """
    t = 0.0  # Initialize time
    while True:
        # Calculate the next sample value using the sine function
        time = np.linspace(t, t + (n_samples - 1) / sample_rate, n_samples)
        samples = amplitude * np.sin(2 * np.pi * frequency * time + phase)
        yield samples
        t += n_samples / sample_rate

def beam_samples_generator(delays, frequency, sample_rate, n_samples = N_SAMPLES):
    t = 0.0  # Initialize time
    offset = np.random.random()
    offset = 0
    while True:
        # Calculate the next sample value using the sine function
        time = np.linspace(t, t + (n_samples - 1) / sample_rate, n_samples)
        samples = np.zeros((N_SENSORS, n_samples), dtype=np.float32)

        for i in range(N_SENSORS):
            total_sa = sample_rate / frequency
            phase = delays[i] * 2*np.pi / total_sa
            #print("phase: ", phase)
            samples[i] = 1.0 * np.sin(2 * np.pi * frequency * time + phase+offset)
        # samples = 1.0 * np.sin(2 * np.pi * frequency * time + phase)
        yield samples
        t += n_samples / sample_rate


from build.lib import pipeline, antenna

import matplotlib.pyplot as plt

class DatasetGenerator(pipeline.Pipeline):

    counter = 0

    sources = []


    def __init__(self):
        super().__init__()

        origo = np.zeros(3)
        self.array = antenna._create_antenna(origo)

        self.Arr = antenna.Array(COLUMNS, ROWS)

        self.sources = []
        # print(self.Arr.steer(2.0, 1.0))
        

        #print("Initiated dataset generator")

    def set_sine_type(self, amplitude, frequency, sample_rate = SAMPLE_RATE, n_samples = N_SAMPLES, phase=0.0):
        self.wave_gen = sine_wave_samples_generator(amplitude, frequency, sample_rate, n_samples, phase)

    def add_sine(self):
        sine = next(self.wave_gen)
        result = np.float32(np.tile(sine, (N_SENSORS, 1)))
        self.store_all(result)

    def add_source(self, azimuth, elevation, frequency, sample_rate = SAMPLE_RATE):
        self.Arr.steer(azimuth, elevation)
        steering_vector = self.Arr.get_delay_vector()
        #print(steering_vector)
        beam_gen = beam_samples_generator(steering_vector, frequency, sample_rate)
        self.sources.append(beam_gen)

    def configure_beam(self, azimuth, elevation, frequency, sample_rate = SAMPLE_RATE):
        self.Arr.steer(azimuth, elevation)
        delays = self.Arr.get_delay_vector()
        self.set_beam_type(delays, frequency, sample_rate)

    def set_beam_type(self, steering_vector, frequency, sample_rate = SAMPLE_RATE, n_samples = N_SAMPLES):
        self.beam_gen = beam_samples_generator(steering_vector, frequency, sample_rate)

    def add_beam(self):
        
        self.store_all(next(self.beam_gen))

    def add_beams(self):
        """Add all beams that are sources

        TODO should the result be computed as mean? at the moment, constructive
        interference may give values greater than 1
        """

        result = np.zeros((N_SENSORS, N_SAMPLES), dtype=np.float32)

        for source in self.sources:
            result += next(source)

        self.store_all(result)

    def create_directed_beam(self, azimuth, elevation):
        """Compute a directed beam based on the steering vector from"""

        self.Arr.steer(azimuth, elevation)
        delays = self.Arr.get_delay_vector()
        

        frames = self.delay_last_frame(delays)

        return frames
    
    def evenly_spaced_points_on_hemisphere(self, N):
        # Calculate elevation angles evenly spaced from 0 to 90 degrees
        elevation = np.linspace(0, 90, N)

        # Calculate azimuth angles evenly spaced in a full circle (360 degrees)
        golden_angle = (1.0 + np.sqrt(5.0)) / 2.0  # Golden angle in radians
        azimuth = np.linspace(0, 2 * np.pi * (1 - 1 / golden_angle), N)

        # Convert elevation and azimuth angles to degrees
        elevation_deg = np.degrees(elevation)
        azimuth_deg = np.degrees(azimuth)

        return elevation_deg, azimuth_deg
    
    def mimo(self, beam):
        n = 50*50
        points = ant.generate_points_half_sphere(n)

        angles = ant.convert_points_to_steering_angles(points)

        image = np.zeros((angles.shape[0], 3))

        for i, (azimuth, elevation) in enumerate(angles):
            # print(azimuth, elevation)
            self.Arr.steer(azimuth, elevation)
            delay = self.Arr.get_delay_vector()
            result = self.delay_last_frame(delay)

            result = result.sum(axis=0)

            result /= N_SENSORS

            result = np.sum(result**2) / result.shape[0]
            image[i] = [azimuth, elevation, result]

        return image, points, ant.convert_points_to_polar_angles(points)


    
    def _mimo(self, beam):
        n = 80
        res = np.linspace(-90, 90, n)
        # res /= 5
        # # res = np.sin(res) * 180
        # res = np.arcsin(res) * 360

        image = np.zeros((n,n))

        delays = np.zeros((n, n, COLUMNS * ROWS))

        this = None




        for y in range(n):
            for x in range(n):
                self.Arr.steer(res[x], res[y])

                delay = self.Arr.get_delay_vector()
  

                result = self.delay_last_frame(delay)


                result = result.sum(axis=0)


                this = result

                result /= N_SENSORS

                result = np.sum(result**4) / result.shape[0]
                image[y, x] = result

        return image, this








    # def lol(self):

    #     n = 5
    #     total = np.zeros(n * N_SAMPLES)

    #     for i in range(N):
    #         # sine = np.zeros(N_SAMPLES, dtype=np.float32)
    #         sine = next(wave_gen)
    #         result = np.float32(np.tile(sine, (N_SENSORS, 1)))

    #         self.store_all(result)
    #         # total[i*N_SAMPLES:(i+1)*N_SAMPLES] = sine
    #         # for k in range(N_SAMPLES):
    #         #     sine[k] = next(wave_gen)
    #     #     print(sine)
    #     # print(result)
    #     # plt.plot(result[0] / 2)
    #     # plt.plot(result[1])
    #     # plt.show()

    #     frames = self.latest()

    #     max_delay = 2

    #     delays = np.linspace(0, max_delay + 1, N_SENSORS, dtype=np.float32)

    #     frames = self.delay_last_frame(delays)
    #     for i in range(N_SENSORS):
    #         plt.plot(frames[i], label=f"Sensor: {i}")

    #     plt.legend()
    #     plt.show()

    #     # print(self.latest())

