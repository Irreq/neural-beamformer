#include <Eigen/Dense>
using namespace Eigen;
void matrixMultiplication(const MatrixXf& A, const MatrixXf& B, MatrixXf& result);

/**
 * Compute the delay in regard to the Z value 
 */
VectorXf compute_delays(const MatrixXf& antenna);

/**
 * Calculate a rotation using linalg
 * TODO explain how it works
 */
inline Matrix3f compute_rotation_matrix(const float azimuth, const float elevation);

/**
 * Helper function to return the middle of the antenna
 */
inline Vector3f find_middle(const MatrixXf& antenna);

/**
 * Place the antenna by positioning the center @ new position
 */
void place_antenna(MatrixXf& antenna,
                   const Vector3f position);

/**
 * Generate a new antenna by specific options and place it @ position
 */
MatrixXf create_antenna(const Vector3f& position,
                        const int columns,
                        const int rows,
                        const float distance);

/**
 * Find out which sensors to use, if not all should be used at the same time.
 */
vector<int> used_sensors(const MatrixXf& antenna, const float distance=DISTANCE);

/**
 * Steer the antenna by creating a steered copy since this operation is irreversibly due to noise
 */
MatrixXf steer(const MatrixXf& antenna, const float azimuth, const float elevation);

/**
 * Generate lookup for many directions of delays for each individual microphone
 */
MatrixXf compute_delays_lookup(const MatrixXf& antenna,
                               const int azimuth_resolution,
                               const int elevation_resolution,
                               const float azimuth_max_angle,
                               const float elevation_max_angle);