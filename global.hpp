#pragma once

#include <cmath>
#include <limits>
#include <memory>
#include <vector>

// usings
using std::make_shared;
using std::shared_ptr;

// constants
const float infinity = std::numeric_limits<float>::infinity();
const float pi = 3.1415926535897932385;

// utility functions
inline float deg2rad(float degree) { return degree / 180.0 * pi; }
inline float rad2deg(float radian) { return radian / pi * 180.0; }


// common headers
#include "interval.hpp"
#include "ray.hpp"
#include "vec3.hpp"

// macro
#define ANTIALIASING_TRUE true
#define ANTIALIASING_FALSE false
