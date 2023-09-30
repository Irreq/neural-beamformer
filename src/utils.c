#include <stdint.h>
#include <limits.h>

float normalize_int32(int32_t value) {
    return value < 0
        ? -((float)value) / INT_MIN
        : ((float)value) / INT_MAX;
}

int32_t denormalize_int32(float normalized_value) {
    return (int32_t)(normalized_value < 0
        ? normalized_value * INT_MIN
        : normalized_value * INT_MAX);
}