#pragma once

#include "global.hpp"

class interval
{
public:
    float min;
    float max;

    interval() : min(+infinity), max(-infinity) {}
    interval(float min, float max) : min(min), max(max) {}
    interval(const interval& a, const interval& b)
        : min(fmin(a.min, b.min)), max(fmax(a.max, b.max)) {}

    bool is_contain(float x) const { return min <= x && x <= max; }
    bool is_surround(float x) const { return min < x && x < max; }
    float clamp(float x)const {
        if (x < min)   return min;
        if (x > max)   return max;
        return x;
    }
    float size() const {
        return max - min;
    }
    interval expand(float delta) const {
        auto t = delta / 2;
        return interval(min - t, max + t);
    }
    void noteRange(float x) {
        if (x < min) {
            min = x;
        }
        else if (x > max) {
            max = x;
        }

    }
    void print()const {
        std::cout << "min-max: " << min << ' ' << max << '\n';
    }
    static const interval empty;
    static const interval universe;
};

const interval empty(+infinity, -infinity);
const interval universe(-infinity, +infinity);
interval operator+(const interval& ival, float displacement) {
    return interval(ival.min + displacement, ival.max + displacement);
}
interval operator+(float displacement, const interval& ival) {
    return ival + displacement;
}