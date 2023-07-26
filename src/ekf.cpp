#include <iostream>
#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Eigen/Dense>

using namespace glm;
using namespace Eigen;

class KalmanFilter3D {
private:
    mat<6, 6, float> A;  // State transition matrix
    mat<6, 6, float> Q;  // Process noise covariance
    mat<3, 6, float> H;  // Measurement matrix
    mat<3, 3, float> R;  // Measurement noise covariance
    mat<6, 6, float> P;  // Error covariance matrix
    vec<6, float> x;     // State vector

public:
    KalmanFilter3D() {
        A = mat<6, 6, float>(1.0f);
        Q = mat<6, 6, float>(0.1f);
        H = mat<3, 6, float>(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                             0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                             0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
        R = mat<3, 3, float>(0.1f);
        P = mat<6, 6, float>(1.0f);
        x = vec<6, float>(0.0f);
    }

    void update(const vec3& measurement) {
        // Prediction
        x = A * x;
        P = A * P * transpose(A) + Q;

        // Kalman gain calculation
        mat3 S = H * P * transpose(H) + R;
        mat<6, 3, float> K = P * transpose(H) * inverse(S);

        // Update step
        vec3 y = measurement - H * x;
        x = x + K * y;
        P = (mat<6, 6, float>(1.0f) - K * H) * P;
    }

    vec3 getState() const {
        return vec3(x);
    }

    vec3 predict(int N) {
        mat<6, 6, float> An = A;
        vec<6, float> xn = x;

        // Apply state transition matrix N times
        for (int i = 0; i < N; ++i) {
            xn = An * xn;
            An = An * A;
        }

        return vec3(xn);
    }
};

// Conversion functions between Eigen::Vector3f and glm::vec3
vec3 toGlmVec3(const Vector3f& v) {
    return vec3(v.x(), v.y(), v.z());
}

Vector3f toEigenVector3f(const vec3& v) {
    return Vector3f(v.x, v.y, v.z);
}

int main() {
    KalmanFilter3D kf;

    // Simulate measurements
    std::vector<Vector3f> measurements = {
        Vector3f(1, 2, 3),
        Vector3f(1.2, 2.4, 3.6),
        Vector3f(0.8, 1.6, 2.8),
        Vector3f(2, 4, 6)
    };

    // Apply Kalman filter to each measurement
    for (const auto& measurement : measurements) {
        kf.update(toGlmVec3(measurement));
        vec3 state = kf.getState();
        std::cout << "Filtered state: " << state.x << ", " << state.y << ", " << state.z << std::endl;

        int N = 2;  // Number of seconds to predict ahead
        vec3 predictedState = kf.predict(N);
        std::cout << "Predicted state (" << N << " seconds ahead): " << predictedState.x << ", " << predictedState.y << ", " << predictedState.z << std::endl;
    }

    return 0;
}
