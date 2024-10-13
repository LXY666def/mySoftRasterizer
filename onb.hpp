#pragma once

#include "global.hpp"

class onb {
public:
    onb(){}
    onb(const vec3& w) { build_from_w(w); }
    onb(const vec3& g, const vec3& t) { build_from_camera(g, t); }

    vec3 operator[](int i) const { return axis[i]; }
    vec3& operator[](int i) { return axis[i]; }

    vec3 u() const { return axis[0]; }
    vec3 v() const { return axis[1]; }
    vec3 w() const { return axis[2]; }

    vec3 local2world(double a, double b, double c) const {
        return a * u() + b * v() + c * w();
    }

    vec3 local2world(const vec3& a) const {
        return a.x() * u() + a.y() * v() + a.z() * w();
    }

    vec3 world2local(const vec3& a) const {
        vec3 result(0);
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                result[i] += axis[i][j] * a[j] * (i==2?-1:1);
            }
        }
        return result;
    }

    void build_from_w(const vec3& w) {
        vec3 unit_w = normalize(w);
        vec3 a = (fabs(unit_w.x()) > 0.9) ? vec3(0, 1, 0) : vec3(1, 0, 0);
        vec3 v = normalize(crossProduct(unit_w, a));
        vec3 u = crossProduct(unit_w, v);
        axis[0] = u;
        axis[1] = v;
        axis[2] = unit_w;
    }
    void build_from_camera(const vec3& g, const vec3& t) {  //left-handed coordinate
        axis[2] = normalize(g);
        axis[0] = normalize(crossProduct(g, t));
        axis[1] = crossProduct(axis[0], axis[2]);
    }

public:
    vec3 axis[3];
};