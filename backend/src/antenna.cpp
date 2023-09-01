#include <iostream>
#include <Eigen/Dense>
#include <math.h>

#define PROPAGATION_SPEED 340.f
#define SAMPLE_RATE 48828.f

using namespace Eigen;

inline float to_radians(float degree)
{
    return degree * M_PI / 180.0;
}

VectorXf compute_delays(const MatrixXf& antenna)
{
    VectorXf delays = antenna.col(2).array() * (SAMPLE_RATE / PROPAGATION_SPEED);

    delays.array() -= delays.minCoeff();
    
    std::cout << "delays  " << delays << std::endl;
    return delays;
}

inline Matrix3f compute_rotation_matrix(const float azimuth, const float elevation)
{
    // MatrixXf rotation_matrix_yaw =
    Matrix3f rotation_matrix_yaw;
    rotation_matrix_yaw <<  cos(azimuth),  0.f, sin(azimuth),
                            0.f,           1.f,          0.f,
                            -sin(azimuth), 0.f, cos(azimuth);

    Matrix3f rotation_matrix_pitch;
    rotation_matrix_pitch << 1.f,            0.f,             0.f,
                             0.f, cos(elevation), -sin(elevation),
                             0.f, sin(elevation), cos(elevation);
    
    return rotation_matrix_yaw * rotation_matrix_pitch;
}

inline Vector3f find_middle(const MatrixXf& antenna)
{
    return antenna.colwise().mean();
}

void place_antenna(MatrixXf& antenna,
                   const Vector3f position)
{
    antenna.rowwise() += position.transpose() - find_middle(antenna).transpose();
}

MatrixXf create_antenna(const Vector3f& position,
                        const int columns,
                        const int rows,
                        const float distance)
{
    float half = distance / 2;
    MatrixXf antenna(rows * columns, 3);

    int index = 0;
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < columns; x++) {
            antenna(index, 0) = x * distance - columns * half + half;
            antenna(index, 1) = y * distance - rows * half + half;
            antenna(index, 2) = 0.f;

            index++;
        }
        
    }

    place_antenna(antenna, position);

    return antenna;
}

int main() {
    Eigen::MatrixXf matrix1(3, 3);
    Eigen::MatrixXf matrix2(2, 3);
    

     // Fill matrix1 with ones
    matrix1.setOnes();

    // Manually set elements in matrix2
    matrix2(0, 0) = 1.0f;
    matrix2(0, 1) = 0.0f;
    matrix2(0, 2) = 0.0f;
    // matrix2(1, 0) = 5.0f;
    // matrix2(1, 1) = 6.0f;
    // matrix2(1, 2) = 7.0f;
    // matrix2(2, 0) = 8.0f;
    // matrix2(2, 1) = 9.0f;
    // matrix2(2, 2) = 10.0f;
    // 
    //matrix1 *= 1;
    // Initialize your matrices with values

    // Perform matrix multiplication
    Eigen::MatrixXf result = matrix2 * matrix1;
    // Eigen::MatrixXf res = 
    std::cout << "Result:" << std::endl << result << std::endl;
    
    Matrix3f combined_rotation_matrix = compute_rotation_matrix(to_radians(45.0), to_radians(0.0));

    //matrix2 *= combined_rotation_matrix.transpose(); 
    // Print the rotation matrix
    std::cout << "Rotation matrix:" << std::endl << matrix2 << std::endl;//combined_rotation_matrix << std::endl;

    MatrixXf middle = (MatrixXf)find_middle(matrix2).transpose();
    //Vector3f middle = find_middle(matrix2);
    // MatrixXf resu = matrix2 - middle.transpose();

    std::cout << "middle" << middle << std::endl;

    std::cout << "matrix" << matrix2.array() << std::endl;

    Vector3f mid = find_middle(matrix2);
    MatrixXf replicated_vector = mid.replicate(matrix2.rows(), 3);

    MatrixXf antenna = create_antenna(Vector3f(0,0,4), 4, 4, 1.0);
    
    antenna = antenna * combined_rotation_matrix.transpose();


    VectorXf delays = compute_delays(antenna);

    std::cout << "delays" << delays << std::endl;
    
    std::cout << "ant" << antenna << std::endl;
    std::cout << "before" << matrix2 << std::endl;
    place_antenna(matrix2, Vector3f(0.f, 0.f, 0.f));
    std::cout << "after" << matrix2 << std::endl;
    //std::cout << "Vector3f" << matrix2.rowwise() + mid.transpose() << std::endl;
    // std::cout << "middle " << matrix2.colwise() * middle.transpose() << std::endl;
    // Subtract the vector from each column of the matrix element-wise
    // Eigen::MatrixXf res = matrix2.array() - replicated_vector.array();
    // std::cout << "resu" << res << std::endl;
    return 0;
}
