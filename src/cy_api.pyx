# cython: language_level=3
# distutils: language=c

# File name: cy_api.pyx
# Description: Cython Module With Added NumPy Support For beamformer
# Author: Irreq
# Date: 2022-12-29

# Main API for C functions

import numpy as np
cimport numpy as np

# It's necessary to call "import_array" if you use any part of the numpy PyArray_* API.
np.import_array()

cimport cython

# To dereference pointers
from cython.operator cimport dereference

import sys
sys.path.insert(0, "") # Access local modules located in . Enables 'from . import MODULE'

# Create specific data-types "ctypedef" assigns a corresponding compile-time type to DTYPE_t.
ctypedef np.float32_t DTYPE_t

# Constants
DTYPE_arr = np.float32

# Define placeholders for other C modules
cdef extern from "beamformer.c":
   void c_module_test()

#cdef extern from "delay.h":
#   void greeting()

# ---- Array Functions ----
#cdef int get_length(DTYPE_t *arr_ptr):
#    """
#    Get the length of an Array
#
#    Arguments:
#        *arr_ptr    Pointer to the array
#
#    Returns:
#        length      int() the length of the array
#    """
#
#    cdef int length
#
#    length = len(arr_ptr)
#
#    return length



#cdef bool local_tests():
#    """Perform local tests on array functions"""
#
#    # Define array
#    cdef np.ndarray[DTYPE_t, ndim=1] arr
#
#    arr = np.array([1], dtype=DTYPE_arr)
#
#    # Define pointer to arr
#
#    cdef DTYPE_t *arr_ptr
#
#    # Regular pointer handling as in C
#    arr_ptr = &arr[0]
#
#    assert get_length(arr_ptr) == 1, "Array length does not add up"

# ---- Entrypoint ----
cdef public void handler():
    # Default entry point for `main.c` program!

    try:
        c_module_test()
    except Exception as e:
        print("Failed calling external module test", e)

    #greeting()
    cdef np.ndarray[DTYPE_t, ndim=1] arr
    
    arr = np.array([1.0, 1.2], dtype=DTYPE_arr)
    print(arr)
    print("It is working!")

