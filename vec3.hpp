#pragma once

#include<cmath>
#include<iostream>
#include<cstdlib>

#include"random.hpp"

class vec3 {
public:
    // member
    float e[4];
    // construction function
    vec3() :e{ 0,0,0,0 } {}
    vec3(float e0, float e1, float e2, float e3 = 0.0) :e{ e0,e1,e2,e3 } {}
    vec3(float t) :e{ t,t,t } {}
    // function
    float x() const { return e[0]; }
    float y() const { return e[1]; }
    float z() const { return e[2]; }
    float w() const { return e[3]; }

    //operator
    vec3 operator-() const { return vec3(-e[0], -e[1], -e[2], -e[3]); }
    float operator[](int i) const { return e[i]; }
    float& operator[](int i) { return e[i]; }
    vec3& operator+=(const vec3& v) {
        e[0] += v.x();
        e[1] += v.y();
        e[2] += v.z();
        e[3] += v.w();
        return *this;
    }
    vec3& operator-=(const vec3& v) {
        e[0] -= v.x();
        e[1] -= v.y();
        e[2] -= v.z();
        e[3] -= v.w();
        return *this;
    }
    vec3& operator*=(const float v) {
        e[0] *= v;
        e[1] *= v;
        e[2] *= v;
        e[3] *= v;
        return *this;
    }
    vec3& operator/=(const float v) {
        auto div_v = 1.0 / v;
        e[0] *= div_v;
        e[1] *= div_v;
        e[2] *= div_v;
        e[3] *= div_v;
        return *this;
    }

    // other
    float norm() const { return std::sqrt(norm2()); }
    float norm2() const { return e[0] * e[0] + e[1] * e[1] + e[2] * e[2]; }

    static vec3 random() {
        return vec3(random_float(), random_float(), random_float());
    }

    static vec3 random(float min, float max) {
        return vec3(random_float(min, max), random_float(min, max), random_float(min, max));
    }
    bool near_zero()const {
        auto s = 1e-8;
        return (fabs(e[0]) < s) && (fabs(e[1]) < s) && (fabs(e[2]) < s);
    }
};

// point3 is just an alias for vec3
using point3 = vec3;
using point2 = vec3;

// vector utility functions
inline std::ostream& operator<<(std::ostream& out, const vec3& v) {
    return out << v.x() << ' ' << v.y() << ' ' << v.z();
}
inline vec3 operator+(const vec3& v1, const vec3& v2) {
    return vec3(v1.x() + v2.x(), v1.y() + v2.y(), v1.z() + v2.z());
}
inline vec3 operator-(const vec3& v1, const vec3& v2) {
    return vec3(v1.x() - v2.x(), v1.y() - v2.y(), v1.z() - v2.z());
}
inline vec3 operator*(const vec3& v1, const float v2) {
    return vec3(v1.x() * v2, v1.y() * v2, v1.z() * v2);
}
inline vec3 operator*(const vec3& v1, const vec3& v2) {
    return vec3(v1[0] * v2[0], v1[1] * v2[1], v1[2] * v2[2]);
}
inline vec3 operator*(const float v2, const vec3& v1) {
    return vec3(v1.x() * v2, v1.y() * v2, v1.z() * v2);
}
inline vec3 operator/(const vec3& v1, const float v2) {
    return vec3(v1.x() / v2, v1.y() / v2, v1.z() / v2);
}
inline float dotProduct(const vec3& v1, const vec3& v2) {
    return v1.x() * v2.x() + v1.y() * v2.y() + v1.z() * v2.z();
}
inline vec3 crossProduct(const vec3& v1, const vec3& v2) {
    return vec3(v1.y() * v2.z() - v1.z() * v2.y(),
        v1.z() * v2.x() - v1.x() * v2.z(),
        v1.x() * v2.y() - v1.y() * v2.x());
}
inline vec3 normalize(const vec3& v) {
    return v / v.norm();
}
inline vec3 max_vec3(const vec3& v1, const vec3& v2) {
    return vec3(fmax(v1[0], v2[0]), fmax(v1[1], v2[1]), fmax(v1[2], v2[2]));
}
inline vec3 min_vec3(const vec3& v1, const vec3& v2) {
    return vec3(fmin(v1[0], v2[0]), fmin(v1[1], v2[1]), fmin(v1[2], v2[2]));
}
// a much faster method than rejection method
// Marsaglia, George. ¡°Choosing a Point from the Surface of a Sphere.¡± Annals of Mathematical Statistics 43 (1972): 645-646.
inline vec3 random_in_sphere_is_wrong() {
    while (true) {
        auto x = random_float(-1, 1);
        auto y = random_float(-1, 1);
        auto s = x * x + y * y;
        if (s < 1) {
            auto root = sqrt(1 - s);
            return vec3(2 * x * root, x * y * root, 1 - 2 * s);
        }
    }
}
inline vec3 random_in_sphere() {
    while (true) {
        auto vec = vec3::random(-1.0, 1.0);
        if (vec.norm2() < 1) {
            return normalize(vec);
        }
    }
}
inline vec3 random_in_disk() {
    while (true) {
        auto p = vec3(random_float(-1, 1), random_float(-1, 1), 0);
        if (p.norm2() < 1) {
            return p;
        }
    }
}
inline vec3 random_in_hemisphere(const vec3& normal) {
    vec3 random_vec = random_in_sphere();
    if (dotProduct(random_vec, normal) > 0.0)
        return random_vec;
    else
        return -random_vec;
}
inline vec3 random_cosine_hemisphere() {
    float pi = 3.1415926535897932385;
    auto r1 = random_float();
    auto r2 = random_float();

    auto phi = 2 * pi * r1;
    auto x = cos(phi) * sqrt(r2);
    auto y = sin(phi) * sqrt(r2);
    auto z = sqrt(1 - r2);

    return vec3(x, y, z);
}
inline vec3 reflect(const vec3& r_in, const vec3& normal) {
    return r_in - 2 * dotProduct(r_in, normal) * normal;
}
inline vec3 refract(const vec3& r_in, const vec3& normal, float relative_n) {
    auto cos = fmin(dotProduct(-r_in, normal), 1.0);
    vec3 r_out_n = relative_n * (r_in + cos * normal);
    vec3 r_out_t = -sqrt(fabs(1.0 - r_out_n.norm2())) * normal;
    return r_out_t + r_out_n;
}