# neural-beamformer

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