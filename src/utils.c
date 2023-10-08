#include <stdint.h>
#include <limits.h>

#define INT24_MIN (-8388608)  // -2^23
#define INT24_MAX (8388607)   // 2^23 - 1


float normalize_int32(int32_t value24Bit) {
    // Mask to keep only the 24 least significant bits
    value24Bit &= 0xFFFFFF;

    // Check the sign bit (bit 23)
    if (value24Bit & 0x800000) {
        // It's a negative number
        // Calculate two's complement
        value24Bit -= 0x1000000;
    }
    
    return value24Bit < 0
        ? -((float)value24Bit) / INT24_MIN
        : ((float)value24Bit) / INT24_MAX;
}

int32_t denormalize_int32(float normalized_value) {
    return (int32_t)(normalized_value < 0
        ? normalized_value * INT_MIN
        : normalized_value * INT_MAX);
}