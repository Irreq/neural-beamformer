# cython: language_level=3
# distutils: language=c

cdef extern from "delay.h":
    int test_delay()