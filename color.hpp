#pragma once

#include "vec3.hpp"
#include "interval.hpp"

#include <iostream>
#include <fstream>

using color = vec3;

static color BLACK = color(0, 0, 0);
static color WHITE = color(1, 1, 1);

inline float linear2gamma(float linear_component) {
    return sqrt(linear_component);
}

inline color interpolate_vec3(float t, const color& c1, const color& c2) {
    return t * c1 + (1 - t) * c2;
}

void write_color(std::ofstream& out, const color& pixel_color) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // replace NaN with 0
    if (r != r) r = 0.0;
    if (g != g) g = 0.0;
    if (b != b) b = 0.0;


    // linear space to gamma space
    /*r = linear2gamma(r);
    g = linear2gamma(g);
    b = linear2gamma(b);*/

    // Write the translated [0,255] value of each color component.
    static const interval intensity(0.0, 0.999);
    out << static_cast<int>(256 * intensity.clamp(r)) << ' '
        << static_cast<int>(256 * intensity.clamp(g)) << ' '
        << static_cast<int>(256 * intensity.clamp(b)) << '\n';
}
