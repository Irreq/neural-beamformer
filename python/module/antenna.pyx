# distutils: language = c++
# distutils: sources = matrix_mult.cpp

# distutils: language = c++
# distutils: sources = matrix_mult.cpp

import numpy as np
cimport numpy as np
cimport eigen as Eigen

cdef extern from "antenna.h":
    void matrixMultiplication(Eigen.MatrixXf& A, Eigen.MatrixXf& B, Eigen.MatrixXf& result)

def multiply_matrices(np.ndarray[np.float64_t, ndim=2] A, np.ndarray[np.float64_t, ndim=2] B):
    cdef Eigen.MatrixXf eigen_A = Eigen.MatrixXfMap(&A[0, 0], A.shape[0], A.shape[1])
    cdef Eigen.MatrixXf eigen_B = Eigen.MatrixXfMap(&B[0, 0], B.shape[0], B.shape[1])
    cdef Eigen.MatrixXf result

    matrixMultiplication(eigen_A, eigen_B, result)

    return np.array(result)

# import numpy as np
# cimport numpy as np

# # It's necessary to call "import_array" if you use any part of the numpy PyArray_* API.
# np.import_array()

# cdef extern from "antenna.h":
#     void matrixMultiplication(float[:, ::1] A, float[:, ::1] B, float[:, ::1] result)

# def multiply_matrices(A, B):
#     cdef int m = A.shape[0]
#     cdef int n = A.shape[1]
#     cdef int p = B.shape[1]

#     cdef float[:, ::1] A_view = A
#     cdef float[:, ::1] B_view = B
#     cdef float[:, ::1] result = np.zeros((m, p), dtype=float)

#     matrixMultiplication(A_view, B_view, result)
#     return result