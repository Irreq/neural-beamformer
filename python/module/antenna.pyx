# cython: language_level=3
# distutils: language=c++
"""# distutils: sources = matrix_mult.cpp"""


import numpy as np
cimport numpy as np

# It's necessary to call "import_array" if you use any part of the numpy PyArray_* API.
np.import_array()


include "config.pxd"
include "antenna.pxd"

cdef VectorXfToNumpy(VectorXf vec):
    result = np.zeros(vec.rows(), dtype=np.float32)
    for i in range(vec.rows()):
        result[i] = vec.coeff(i)
    return result

cdef MatrixXfToNumpy(MatrixXf matrix):
    result = np.zeros((matrix.rows(),matrix.cols())) # create nd array
    # Fill out the nd array with MatrixXf elements 
    for row in range(matrix.rows()): 
        for col in range(matrix.cols()): 
            result[row, col] = matrix.coeff(row, col)

    return result

# def _steer()



cdef ok():
    cdef Vector3f pos = Vector3f(0.0, 0.0, 0.0)

    cdef MatrixXf antenna = create_antenna(pos, COLUMNS, ROWS, DISTANCE)

    cdef MatrixXf steered = steer(antenna, 10.0, 5.0)



    # me = antenna
    # result = np.zeros((me.rows(),me.cols())) # create nd array
    # print(result.shape)
    # # Fill out the nd array with MatrixXf elements 
    # for row in range(me.rows()): 
    #     for col in range(me.cols()): 
    #         result[row, col] = me.coeff(row, col)

    cdef VectorXf delays = compute_delays(steered)

    result = np.zeros((ROWS, COLUMNS), dtype=np.float32)

    i = 0
    for y in range(ROWS):
        for x in range(COLUMNS):
            result[y, x] = delays.coeff(i)
            i += 1



    return result

# if i have a cpp class known as A that i have imported to cython.

# How can i call a cython function with that class? that is:

# cdef bar(A a):
#     a.do_domething() # This does not work gives: Cannot convert Python object argument to type 'A'

# cdef foo(): 
#     cdef A a = A()

#     a.do_domething() # This works

# ctypedef np.ndarray[np.float32_t, ndim=1, order="c"]

# cimport eigen

# np.ndarray[np.float32_t, ndim=1, mode="c"]

ORIGO = np.zeros(3, dtype=np.float32)

cdef class Array:

    cdef MatrixXf antenna
    cdef Vector3f pos

    cdef MatrixXf steered

    cdef object antenna_repr

    def __init__(self, int rows, int cols, float distance=DISTANCE, position = ORIGO):
        self.pos = Vector3f(position[0], position[1], position[2])
        self.antenna = create_antenna(self.pos, cols, rows, distance)
        self.toNumpy()

    cdef public np.ndarray toNumpy(self):
        self.antenna_repr = np.zeros((self.antenna.rows(), self.antenna.cols()), dtype=np.float32)

        for row in range(self.antenna.rows()):
            for col in range(self.antenna.cols()):
                self.antenna_repr[row, col] = self.antenna.coeff(row, col)

        return self.antenna_repr

    def __getitem__(self, int idx):
        if not (-self.antenna.rows() < idx < self.antenna.rows()):
            raise IndexError("Out of bounds error")

        return self.antenna_repr[idx]

    def steer(self, float azimuth, float elevation):
        self.steered = steer(self.antenna, azimuth, elevation)

        return MatrixXfToNumpy(self.steered)

    def get_delay_vector(self):
        return VectorXfToNumpy(compute_delays(self.steered))




cdef object bar(MatrixXf antenna):
    print(antenna.cols(), 276347263476327)

    return np.zeros(5)


# cpdef np.ndarray toNumpy(MatrixXf& antenna):
#     result = np.zeros((antenna.rows(), antenna.cols()), dtype=np.float32)

#     for row in range(antenna.rows()):
#         for col in range(antenna.cols()):
#             result[row, col] = antenna.coeff(row, col)

#     return result

# Antenna creation

cdef np.ndarray cy_create_antenna(np.ndarray position):
    cdef Vector3f pos = Vector3f(position[0], position[1], position[2])
    cdef MatrixXf antenna = create_antenna(pos, COLUMNS, ROWS, DISTANCE)

    # return toNumpy(antenna)

    # print(bar(antenna))

    result = np.zeros((antenna.rows(), antenna.cols()), dtype=np.float32)

    for row in range(antenna.rows()):
        for col in range(antenna.cols()):
            result[row, col] = antenna.coeff(row, col)

    return result

def _create_antenna(position):
    """Wrapper for cy_create_antenna"""
    return cy_create_antenna(position)


def test():
    #a = Array(2, 2)
    # print(a.toNumpy(), 696969, a[2])
    # cdef Eigen.MatrixXf a 
    # print(arr())
    antenna = ok()
    # print(antenna)
    # print("Tested Eigen array")
    return antenna
