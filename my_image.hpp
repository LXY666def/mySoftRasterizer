#ifndef MY_IMAGE_H
#define MY_IMAGE_H

// Disable strict warnings for this header from the Microsoft Visual C++ compiler.
#ifdef _MSC_VER
    #pragma warning (push, 0)
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.hpp"
#include "interval.hpp"
#include "color.hpp"

#include <cstdlib>
#include <iostream>

#define IMAGE_USAGE_UNDEFINED 0
#define IMAGE_USAGE_READ 1
#define IMAGE_USAGE_WRITE 2

class my_image {
  public:
    my_image(int w, int h):image_width(w),image_height(h),image_usage(IMAGE_USAGE_WRITE){}
    my_image(const char* image_filename, int usage = 0):image_usage(IMAGE_USAGE_READ) {
        if(usage == 0){ // rgb
            auto filename = std::string(image_filename);
            if (!load(filename, usage)) {
                std::cerr << "ERROR: Could not load image file '" << image_filename << "'.\n";
            }
        }else if (usage == 1) { // hdr
            auto filename = std::string(image_filename);
            if (!load(filename, usage)) {
                std::cerr << "ERROR: Could not load hdr file '" << image_filename << "'.\n";
            }
        }
    }

    ~my_image() { STBI_FREE(data_U8); }

    bool load(const std::string filename, int usage) {
        // Loads image data from the given file name. Returns true if the load succeeded.
        auto n = bytes_per_pixel; // Dummy out parameter: original components per pixel
        if(usage == 0)
            data_U8 = stbi_load(filename.c_str(), &image_width, &image_height, &n, bytes_per_pixel);
        bytes_per_scanline = image_width * bytes_per_pixel;
        return data_U8 != nullptr;
    }

    int width()  const { return (data_U8 == nullptr) ? 0 : image_width; }
    int height() const { return (data_U8 == nullptr) ? 0 : image_height; }

    const unsigned char* pixel_data(int x, int y) const {
        // Return the address of the three bytes of the pixel at x,y (or magenta if no data).
        static unsigned char magenta[] = { 255, 0, 255 };
        if (data_U8 == nullptr) return magenta;

        x = clamp(x, 0, image_width);
        y = clamp(y, 0, image_height);

        return data_U8 + y*bytes_per_scanline + x*bytes_per_pixel;
    }
    const unsigned char* texture_data(float u, float v) const {
        u = interval(0, 1).clamp(u);
        v = interval(0, 1).clamp(v);

        return pixel_data(u *(image_width-1), (1-v)*(image_height-1));
    }
    void outputImage(const char* path, color* data) {
        if (image_usage != IMAGE_USAGE_WRITE) {
            std::cerr << "image_usage error\n";
            return;
        }
        std::ofstream output;
        output.open(path, std::ios::out);
        output << "P3\n" << image_width << ' ' << image_height << "\n255\n";
        for (int i = 0; i < image_height; i++) {
            for (int j = 0; j < image_width; j++) {
                write_color(output, data[image_width * (image_height - i - 1) + j]);
            }
        }
        output.close();
    }
    void outputImage(const char* path, float* data) {
        if (image_usage != IMAGE_USAGE_WRITE) {
            std::cerr << "image_usage error\n";
            return;
        }
        std::ofstream output;
        output.open(path, std::ios::out);
        output << "P2\n" << image_width << ' ' << image_height << "\n255\n";
        for (int i = 0; i < image_height; i++) {
            for (int j = 0; j < image_width; j++) {
                auto gray = 255 - (int)data[image_width * (image_height - i - 1) + j];  //deeper, darker
                //auto gray = (int)data[image_width * (image_height - i - 1) + j];      //deeper, brighter
                output << gray << ' ';
            }
            output << '\n';
        }
        output.close();
    }

  private:
    const int bytes_per_pixel = 3;
    unsigned char *data_U8;
    float* data_hdr;
    color* data_RGB;
    int image_width, image_height;
    int bytes_per_scanline;
    int image_usage = IMAGE_USAGE_UNDEFINED;

    static int clamp(int x, int low, int high) {
        // Return the value clamped to the range [low, high).
        if (x < low) return low;
        if (x < high) return x;
        return high - 1;
    }
};

// Restore MSVC compiler warnings
#ifdef _MSC_VER
    #pragma warning (pop)
#endif

#endif