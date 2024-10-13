#pragma once

#include"my_image.hpp"

class Texture {
public:
    virtual ~Texture() = default;
    virtual color value(float u, float v, const point3& p) const = 0;
};

class Texture_solid : public Texture {
public:
    Texture_solid(color c) : color_value(c) {}

    Texture_solid(float red, float green, float blue) :color_value(color(red, green, blue)) {}

    color value(float u, float v, const point3& p)const override {

        return color_value / 255.0;
    }
private:
    color color_value;
};

class Texture_checker : public Texture {
public:
    Texture_checker(float _scale, shared_ptr<Texture> _even, shared_ptr<Texture> _odd)
        : inv_scale(1.0 / _scale), even(_even), odd(_odd) {}
    Texture_checker(float _scale, color c1, color c2)
        : inv_scale(1.0 / _scale), even(make_shared<Texture_solid>(c1)), odd(make_shared<Texture_solid>(c2)) {}

    color value(float u, float v, const point3& p)const override {
        auto int_x = static_cast<int>(std::floor(inv_scale * p.x()));
        auto int_y = static_cast<int>(std::floor(inv_scale * p.y()));
        auto int_z = static_cast<int>(std::floor(inv_scale * p.z()));

        bool isEven = (int_x + int_y + int_z) % 2 == 0;

        return isEven ? even->value(u, v, p) : odd->value(u, v, p);
    }
private:
    float inv_scale;
    shared_ptr<Texture> even;
    shared_ptr<Texture> odd;
};

class Texture_image : public Texture {
public:
    Texture_image(const char* filename) :image(filename) {}

    color value(float u, float v, const point3& p) const override {
        // If we have no texture data
        if (image.height() < 1)
            return color(0, 1, 1);
        const unsigned char* pColor = image.texture_data(u, v);
        auto result = color(pColor[0] / 255.0, pColor[1] / 255.0, pColor[2] / 255.0);
        return result;
    }

private:
    my_image image;
};