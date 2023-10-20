# cython: language_level=3
# distutils: language=c++

include "config.pxd"

# from libcpp.vector cimport vector

cdef extern from "Eigen/Dense" namespace "Eigen":
    cdef cppclass MatrixXf:
        MatrixXf() except +

        MatrixXf(int d1, int d2) except +
        MatrixXf(MatrixXf other) except +
        void setOnes()
        void setZero()
        int rows()
        int cols()
        float coeff(int, int)

        float *data()
        float& element "operator()"(int row, int col)

    cdef cppclass VectorXf:
        VectorXf() except +
        void setOnes()
        void setZero()
        int rows()
        int cols()
        float coeff(int)
    cdef cppclass Vector3f:
        Vector3f() except +
        Vector3f(float a, float b, float c)

cdef extern from "antenna.h":
    VectorXf compute_delays(const MatrixXf& antenna)
    void place_antenna(MatrixXf& antenna, const Vector3f position)
    MatrixXf create_antenna(const Vector3f& position,
                        const int columns,
                        const int rows,
                        const float distance)
    # vector[int] used_sensors(const MatrixXf& antenna, const float distance=DISTANCE)
    MatrixXf steer(const MatrixXf& antenna, const float azimuth, const float elevation)
    MatrixXf compute_delays_lookup(const MatrixXf& antenna,
                               const int azimuth_resolution,
                               const int elevation_resolution,
                               const float azimuth_max_angle,
                               const float elevation_max_angle)
