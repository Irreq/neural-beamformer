# cython: language_level=3
# distutils: language=c++
"""# distutils: sources = matrix_mult.cpp"""


import numpy as np
cimport numpy as np


include "config.pxd"
include "antenna.pxd"

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

    result = np.zeros((COLUMNS, ROWS), dtype=np.float32)

    i = 0
    for y in range(ROWS):
        for x in range(COLUMNS):
            result[y, x] = delays.coeff(i)
            i += 1



    return result

# Antenna creation

cdef np.ndarray cp_create_antenna(np.ndarray position):
    cdef Vector3f pos = Vector3f(position[0], position[1], position[2])
    cdef MatrixXf antenna = create_antenna(pos, COLUMNS, ROWS, DISTANCE)

    result = np.zeros((antenna.rows(), antenna.cols()), dtype=np.float32)

    for row in range(antenna.rows()):
        for col in range(antenna.cols()):
            result[row, col] = antenna.coeff(row, col)

    return result

def _create_antenna(position):
    """Wrapper for cp_create_antenna"""
    return cp_create_antenna(position)


def test():
    # cdef Eigen.MatrixXf a 
    # print(arr())
    antenna = ok()
    # print(antenna)
    print("Tested Eigen array")
    return antenna