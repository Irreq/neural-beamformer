# eigen.pxd
cdef extern from "Eigen/Dense":
    cdef cppclass MatrixXf:
        MatrixXf() except +

        # MatrixXf(int d1, int d2) except +
        MatrixXf(MatrixXf other) except +
        # Define the properties and methods you need
        MatrixXf(int rows, int cols) except +
        float& operator()(int i, int j) except +
        int cols()
        int rows()
        # Add more declarations as needed