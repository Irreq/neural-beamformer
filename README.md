# neural-beamformer

A CNN based beamformer.

The program contains methods for generating, transforming and rotate theoretical antennas in 3D space. A simple framework
exists for generating antenna arrays. 

The beamforming process consists of a delay and sum process. Each signal gathered by a sensor will be delayed both whole samples and later delayed by fractional samples.
It is. In order to delay a signal, one must first store the previous signals in time. This is achieved by a circular buffer. The delay function may then gather the signals
back a certain amount of time specified by the circular buffer. The signal may then be shifted in time and later stored in the sum signal.
The summarized signal is later divided by the number of elements to get the mean power for the specific direction. This is done repeatedly in the desired directions. That is, if
a 20x20 pixel image is wanted, a total of:

Y * X * N_SENSORS * N_SAMPLES

This quicly grows to a very large operation.
The Auxilary space for this function is O(N^2) whereas the time is in the order of O(N^4). shortcuts can however be made, due to ~physics~ the optimum distance between
elements to get the best result is no more than half the wavelength, thanks to the Nyqvist theorem. One may therefore skip elements based on which Frequency region you want to monitor.
The current antenna uses fixed distance between elements, but research is being made for creating antennas with scattered elements for better utilzation of the elements. This is to reduce 
product cost but also reduce the amount of computation required to perform a successful beamforming of a wave.

The antenna here has around 2.0cm between elements which is 0.02m which allows the antenna to reach optimum efficiency at c = \lambda * f which for us results in:
340 / (0.02 * 2) = f = 8500Hz = 8.5kHz which means that frequencies greater than that will result in grating lobes due to the Nyqvist theorem.
It ts therefore crucial to construct an antenna which matches the requirements necessary to listen in that area. However since we are limited by 8.5kHz we does not need to sample faster than 
twice that thanks to the nyqvist theorem. So we save a lot of computation by having more time to perform the beamforming process.
this also means that the required distance between elements can be using the following chart:

d = 0.04 (every other element) = 4.25kHz
d = 0.06 (every third element) = 2.83kHz

Every _ element:

1 : 8.500kHz
2 : 4.245kHz
3 : 2.833kHz
4 : 2.125kHz
5 : 1.700kHz
6 : 1.417kHz
7 : 1.214kHz


We are to create a program that achieves the following:

A constant UDP transfer of signals are received. This data must be stored for a short duration of time so that the worker functions may operate on them. The worker functions have variables that control their behaviour, but whatever they do, they store their result for short duration of time (think that they sits between the incoming stream and the output). Somewhere else, a user might change a variable in a GUI, and this will propagate to the worker function and they will perform their new task. One such operation might be to direct the incoming stream to a loudspeaker. And another might be calculate amplitude and output it on a screen.



A FIR-based delay-and-sum beamformer with SIMD support.

## Frequency Response
![Response](https://github.com/Irreq/neural-beamformer/blob/main/assets/main_lobe.png)



## Installation

Clone this project

    git clone https://github.com/Irreq/neural-beamformer.git

Go to the directory

    cd neural-beamformer

To use the delay functions, one must compile the code. If AVX instructions are available
the user can use `simd_delay` instead of `naive_delay` which is oftentimes
100x faster than python3 `numpy.convolve()`.

Compile using `make`

    make

A shared object has now been created inside `lib/` known as `libdelay.so` which can be used to delay signals.
