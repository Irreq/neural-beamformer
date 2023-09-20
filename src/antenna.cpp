#include <iostream>
#include <Eigen/Dense>
#include <math.h>

#include <vector>

#include "config.h"

#define PROPAGATION_SPEED 340.f
#define SAMPLE_RATE 48828.f
#define DISTANCE 0.02

#define SKIP_N_MICS 2

using namespace Eigen;
using namespace std;

void matrixMultiplication(const MatrixXf& A, const MatrixXf& B, MatrixXf& result) {
    result = A * B;
}

/**
 * Simple way to convert degree to radians
 */
inline float to_radians(float degree)
{
    return degree * M_PI / 180.0;
}

/**
 * Compute the delay in regard to the Z value 
 */
VectorXf compute_delays(const MatrixXf& antenna)
{
    VectorXf delays = antenna.col(2).array() * (SAMPLE_RATE / PROPAGATION_SPEED);

    delays.array() -= delays.minCoeff();
    
    return delays;
}


/**
 * Calculate a rotation using linalg
 * TODO explain how it works
 */
inline Matrix3f compute_rotation_matrix(const float azimuth, const float elevation)
{
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

/**
 * Helper function to return the middle of the antenna
 */
inline Vector3f find_middle(const MatrixXf& antenna)
{
    return antenna.colwise().mean();
}

/**
 * Place the antenna by positioning the center @ new position
 */
void place_antenna(MatrixXf& antenna,
                   const Vector3f position)
{
    antenna.rowwise() += position.transpose() - find_middle(antenna).transpose();
}

/**
 * Generate a new antenna by specific options and place it @ position
 */
MatrixXf create_antenna(const Vector3f& position,
                        const int columns,
                        const int rows,
                        const float distance)
{
    float half = distance / 2;
    MatrixXf antenna(rows * columns, 3); // (id, X|Y|Z)

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

/**
 * Find out which sensors to use, if not all should be used at the same time.
 */
vector<int> used_sensors(const MatrixXf& antenna, const float distance=DISTANCE)
{
    vector<int> used;

    MatrixXf normalized = antenna / distance;
    normalized.rowwise() -= normalized.colwise().minCoeff();
    normalized = normalized.array().round().matrix();

    int x, y;
    for (int i = 0; i < normalized.rows(); i++) {
        x = normalized(i, 0);
        y = normalized(i, 1);

        // Only add if its every SKIP_N_MICS or each one if else 
        if ((((x % SKIP_N_MICS) == 0) && ((y % SKIP_N_MICS) == 0)) || (SKIP_N_MICS <= 0))
        {
            used.push_back(i);
        }
    }

    return used; 
}

/**
 * Steer the antenna by creating a steered copy since this operation is irreversibly due to noise
 */
MatrixXf steer(const MatrixXf& antenna, const float azimuth, const float elevation)
{
    MatrixXf steered = antenna;

    steered *= compute_rotation_matrix(to_radians(azimuth), to_radians(elevation)).transpose();

    return steered;
}

/**
 * Generate lookup for many directions of delays for each individual microphone
 */
MatrixXf compute_delays_lookup(const MatrixXf& antenna,
                               const int azimuth_resolution,
                               const int elevation_resolution,
                               const float azimuth_max_angle,
                               const float elevation_max_angle)
{
    MatrixXf delays(azimuth_resolution * elevation_resolution, antenna.rows());

    VectorXf azimuth_angles = VectorXf::LinSpaced(azimuth_resolution, -azimuth_max_angle, azimuth_max_angle);
    VectorXf elevation_angles = VectorXf::LinSpaced(elevation_resolution, -elevation_max_angle, elevation_max_angle);


    int i = 0;
    for (float y : elevation_angles)
    {
        for (float x : azimuth_angles)
        {
            delays.row(i++) = compute_delays(steer(antenna, x, y));
        }
    }
    
    return delays;

}

// class Array {
//     public:
//         vector get_data() {
//             return 
//         }
// }

// int main (int argc, char *argv[]) {
//     MatrixXf antenna = create_antenna(Vector3f(0,0,0), 8, 8, DISTANCE);

        
//     std::cout << "antenna" << antenna << std::endl;

//      Matrix3f combined_rotation_matrix = compute_rotation_matrix(to_radians(45.0), to_radians(0.0));

//      //antenna = antenna * combined_rotation_matrix.transpose();
     
//      MatrixXf steered = steer(antenna, 45.0, 0.f);

//      std::cout << "antenna" << antenna << std::endl;

//      std::cout << "steered" << steered << std::endl;

//     vector<int> used = used_sensors(antenna);

//     std::cout << "used" << std::endl;

//     for(int id : used) {
//         cout << id << "\n";
        
//     }

//     cout << endl;


//     int azimuth_resolution = 51;
//     int elevation_resolution = 51;
//     float AZIMUTH_MAX_ANGLE = 80.0;
//     float ELEVATION_MAX_ANGLE = 80.0;
//     MatrixXf delays = compute_delays_lookup(antenna, azimuth_resolution, elevation_resolution, AZIMUTH_MAX_ANGLE, ELEVATION_MAX_ANGLE);
//     std::cout << delays << std::endl;

//     return 0;
// }

// int _main() {
//     Eigen::MatrixXf matrix1(3, 3);
//     Eigen::MatrixXf matrix2(2, 3);
//     
//
//      // Fill matrix1 with ones
//     matrix1.setOnes();
//
//     // Manually set elements in matrix2
//     matrix2(0, 0) = 1.0f;
//     matrix2(0, 1) = 0.0f;
//     matrix2(0, 2) = 0.0f;
//     // matrix2(1, 0) = 5.0f;
//     // matrix2(1, 1) = 6.0f;
//     // matrix2(1, 2) = 7.0f;
//     // matrix2(2, 0) = 8.0f;
//     // matrix2(2, 1) = 9.0f;
//     // matrix2(2, 2) = 10.0f;
//     // 
//     //matrix1 *= 1;
//     // Initialize your matrices with values
//
//     // Perform matrix multiplication
//     Eigen::MatrixXf result = matrix2 * matrix1;
//     // Eigen::MatrixXf res = 
//     std::cout << "Result:" << std::endl << result << std::endl;
//     
//     Matrix3f combined_rotation_matrix = compute_rotation_matrix(to_radians(45.0), to_radians(0.0));
//
//     //matrix2 *= combined_rotation_matrix.transpose(); 
//     // Print the rotation matrix
//     std::cout << "Rotation matrix:" << std::endl << matrix2 << std::endl;//combined_rotation_matrix << std::endl;
//
//     MatrixXf middle = (MatrixXf)find_middle(matrix2).transpose();
//     //Vector3f middle = find_middle(matrix2);
//     // MatrixXf resu = matrix2 - middle.transpose();
//
//     std::cout << "middle" << middle << std::endl;
//
//     std::cout << "matrix" << matrix2.array() << std::endl;
//
//     Vector3f mid = find_middle(matrix2);
//     MatrixXf replicated_vector = mid.replicate(matrix2.rows(), 3);
//
//     MatrixXf antenna = create_antenna(Vector3f(0,0,4), 4, 4, 1.0);
//     
//     antenna = antenna * combined_rotation_matrix.transpose();
//
//
//     VectorXf delays = compute_delays(antenna);
//
//     std::cout << "delays" << delays << std::endl;
//     
//     std::cout << "ant" << antenna << std::endl;
//     std::cout << "before" << matrix2 << std::endl;
//     place_antenna(matrix2, Vector3f(0.f, 0.f, 0.f));
//     std::cout << "after" << matrix2 << std::endl;
//     //std::cout << "Vector3f" << matrix2.rowwise() + mid.transpose() << std::endl;
//     // std::cout << "middle " << matrix2.colwise() * middle.transpose() << std::endl;
//     // Subtract the vector from each column of the matrix element-wise
//     // Eigen::MatrixXf res = matrix2.array() - replicated_vector.array();
//     // std::cout << "resu" << res << std::endl;
//     //
//     std::cout << "message" << std::endl;
//     return 0;
// }
