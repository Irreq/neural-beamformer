# cython: language_level=3
# distutils: language=c

cimport pipeline

from config cimport *

cdef ring_buffer *rb

import numpy as np
cimport numpy as np

import cython
cimport cython

include "config.h.pxd"

# It's necessary to call "import_array" if you use any part of the numpy PyArray_* API.
np.import_array()


cdef class _PipeLine:
    cdef ring_buffer *rb
    cdef public bint running
    cdef public np.ndarray frame

    def __cinit__(self):
        # Initialize the out_array in the constructor
        self.rb = pipeline.create_ring_buffer()
        self.running = True
        self.frame = np.zeros((N_SENSORS, BUFFER_LENGTH), dtype=np.float32)
        
    @cython.boundscheck(False)
    @cython.wraparound(False)
    def get_data(self, np.ndarray[np.float32_t, ndim=2, mode="c"] out):
        """Copy the ringbuffer into data"""
        read_buffer_all(self.rb, <float (*)[BUFFER_LENGTH]> out.data)
    
    @cython.boundscheck(False)
    @cython.wraparound(False)
    def store(self, np.ndarray[np.float32_t, ndim=1, mode="c"] arr):
        """Store a vector to the ringbuffer"""
        write_buffer_single(self.rb, <float (*)> arr.data)

    @cython.boundscheck(False)
    @cython.wraparound(False)
    def latest(self) -> np.ndarray:
        """Receive the latest samples into a buffer

        """
        read_buffer_all(self.rb, <float (*)[BUFFER_LENGTH]> self.frame.data)
        return self.frame

class Pipeline(_PipeLine):
    def __init__(self):
        super().__init__()
