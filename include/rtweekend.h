#ifndef RTWEEKEND_H
#define RTWEEKEND_H

#include <cmath> // NOLINT
#include <cstdlib>
#include <limits>
#include <memory>
#include <random>

// Defines
#define __F_IN__
#define __F_IN_OPT__
#define __F_OUT__
#define __F_OUT_OPT__
#define __F_INOUT__
#define __F_INOUT_OPT__

// Usings

using std::make_shared;
using std::shared_ptr;
using std::sqrt;

// Constants

const double INF = std::numeric_limits<double>::infinity();
const double PI = 3.1415926535897932385;

// Util functions

inline double degrees_to_radians(double degrees) {
    return degrees * PI / 180.0;
}

/// @brief  Returns a random double in [0, 1).
/// @return A random double
inline double random_double() {
    return rand() / (RAND_MAX + 1.0);
}

/// @brief Returns a random double in [min, max)
/// @param min Minimum value to be generated
/// @param max Maximum value to be generated
/// @return A random double
inline double random_double(double min, double max) {
    return min + (max - min) * random_double();
}

/// @brief  Returns a random double in [0, 1).
/// @return A random double
inline double random_double2() {
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}

/// @brief Returns a random double in [min, max)
/// @param min Minimum value to be generated
/// @param max Maximum value to be generated
/// @return A random double
inline double random_double2(double min, double max) {
    return min + (max - min) * random_double2();
}

/// @brief Clamps down a value to between two other values.
/// @param x The value to be clamped
/// @param min The minimum value to clamp to
/// @param max The maximum value to clamp to
/// @return The clamped value
inline double clamp(double x, double min, double max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

/// @brief Random integer in [min, max]
/// @param min minimum value
/// @param max maximum value
/// @return A random integer
inline int random_int(int min, int max) {
    return static_cast<int>(random_double2(min, max + 1));
}

// Common Headers

#include "ray.h"
#include "vec3.h"

#endif