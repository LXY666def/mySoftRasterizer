#pragma once

#include"global.hpp"
#include"color.hpp"
#include"triangle.hpp"
#include"camera.hpp"
#include"array"
#include"VertexShader.hpp"
#include"FragmentShader.hpp"
#include"hdri.hpp"

enum Primitive {
    __vert   = 1,
    __line   = 2,
    __mesh   = 3
};

class Rasterizer {
public:
    int width;
    int height;
    std::vector<color>& FrameBuffer();
    std::vector<float>& DepthBuffer();

    Rasterizer(const CameraInfo&);
    void render(const VERT_SHADER&, const FRAG_SHADER&, std::vector<Triangle*>, enum Primitive);
    float* getDepthBuffer() { return depth_buffer.data(); }
    void setBackGround(const Camera&, const HDRI& hdri);
private:
    std::vector<color> frame_buffer;
    std::vector<float> depth_buffer;
    int cameraType;

    void drawLines(const point2&, const point2&, const color&);
    void drawTriangle(const outVertShaderPayload&, const FRAG_SHADER&);
    void setPixel(int, int, const color&);
    
};

Rasterizer::Rasterizer(const CameraInfo& Info) :width(Info.width), height(Info.height), cameraType(Info.type) {
    frame_buffer.resize(width * height, BLACK);
    depth_buffer.resize(width * height, 255.999);
}
inline std::vector<color>& Rasterizer::FrameBuffer() {
    return frame_buffer;
}
inline std::vector<float>& Rasterizer::DepthBuffer() {
    return depth_buffer;
}
void Rasterizer::drawLines(const point2& p0, const point2& p1, const color& line_color){
    auto x0 = p0.x();
    auto y0 = p0.y();
    auto x1 = p1.x();
    auto y1 = p1.y();
    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx = x1 - x0;
    int dy = y1 - y0;
    int derror2 = std::abs(dy) * 2;
    int error2 = 0;
    int y = y0;
    for (int x = x0; x <= x1; x++) {
        if (steep) {
            setPixel(y, x, line_color);
        }
        else {
            setPixel(x, y, line_color);
        }
        error2 += derror2;
        if (error2 > dx) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}
inline void Rasterizer::setPixel(int w, int h, const color& _color) {
    if (w<0 || h<0 || w>width - 1 || h>height - 1)
        return;
    frame_buffer[h * width + w] = _color;
}
inline void Rasterizer::setBackGround(const Camera& cam, const HDRI& hdri) {
    auto r = 1 / cam.inv_near_r;
    auto t = 1 / cam.inv_near_t;
    auto coord = cam.getOnb();
    auto origin_left_bottom = cam.getZNear() * coord.w() - coord.u() * r - coord.v() * t;

    auto delta_u = coord.u() * 2 * r / width;
    auto delta_v = coord.v() * 2 * t / height;

    auto pixel_origin = origin_left_bottom + 0.5 * delta_u + 0.5 * delta_v;
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            auto current_dir = pixel_origin + i * delta_u + j * delta_v;
            frame_buffer[j * width + i] = hdri.queryColor(normalize(current_dir));
        }
    }

}
void Rasterizer::render(const VERT_SHADER& vertShader, const FRAG_SHADER& fragShader, std::vector<Triangle*> TriangleList, enum Primitive primitive = __mesh) {
    // background
    // BLACK

    int total = TriangleList.size();
    int index = 0;
    // __line*************************************
    // wait to be inplemented
    if (primitive == __line) {
        for (auto t : TriangleList) {
            for (int i = 0; i < 3; i++) {
                outVertShaderPayload payload = vertShader.processVertData(t);
                drawLines(payload.triangle.vertex[i], payload.triangle.vertex[(i + 1) % 3], color(225, 0, 0));
            }
        }
        return;
    }
    // __mesh*************************************
    if(primitive == __mesh){
        for (auto t : TriangleList) {
            outVertShaderPayload payload = vertShader.processVertData(t);
            drawTriangle(payload, fragShader);
            std::cout << "\rdraw " << (++index) << " / " << total;
        }
        std::cout << std::endl;
    }
}
static bool insideTriangle(float w, float h, const vec3* v) {
    point3 p(w, h, 0);
    auto v01 = v[1] - v[0], v0p = p - v[0];
    auto v12 = v[2] - v[1], v1p = p - v[1];
    auto v20 = v[0] - v[2], v2p = p - v[2];

    float f01 = v01[0] * v0p[1] - v01[1] * v0p[0];
    float f12 = v12[0] * v1p[1] - v12[1] * v1p[0];
    float f20 = v20[0] * v2p[1] - v20[1] * v2p[0];
    if ((f01 >= 0 && f12 >= 0 && f20 >= 0) || (f01 <= 0 && f12 <= 0 && f20 <= 0)) {
        return true;
    }
    return false;
}
static std::array<float, 3> computeBarycentric(float x, float y, const vec3* v) {
    float c1 = (x * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * y + v[1].x() * v[2].y() - v[2].x() * v[1].y()) / (v[0].x() * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * v[0].y() + v[1].x() * v[2].y() - v[2].x() * v[1].y());
    float c2 = (x * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * y + v[2].x() * v[0].y() - v[0].x() * v[2].y()) / (v[1].x() * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * v[1].y() + v[2].x() * v[0].y() - v[0].x() * v[2].y());
    float c3 = (x * (v[0].y() - v[1].y()) + (v[1].x() - v[0].x()) * y + v[0].x() * v[1].y() - v[1].x() * v[0].y()) / (v[2].x() * (v[0].y() - v[1].y()) + (v[1].x() - v[0].x()) * v[2].y() + v[0].x() * v[1].y() - v[1].x() * v[0].y());
    return std::array<float, 3>{c1, c2, c3};
}
static vec3 barycentricInterpolate(const std::array<float, 3>& barycentric, const vec3* attribute) {
    return barycentric[0] * attribute[0] + barycentric[1] * attribute[1] + barycentric[2] * attribute[2];
}
void Rasterizer::drawTriangle(const outVertShaderPayload& vertPayload, const FRAG_SHADER& fragShader) {
    auto& t = vertPayload.triangle;
    // Find out the bounding box of current triangle.
    float xmin = std::min(std::min(t[0].x(), t[1].x()), t[2].x());
    float ymin = std::min(std::min(t[0].y(), t[1].y()), t[2].y());
    float xmax = std::max(std::max(t[0].x(), t[1].x()), t[2].x());
    float ymax = std::max(std::max(t[0].y(), t[1].y()), t[2].y());

    xmin = interval(0, width-1).clamp((int)std::floor(xmin));
    ymin = interval(0, height-1).clamp((int)std::floor(ymin));
    xmax = interval(0, width-1).clamp((int)std::ceil(xmax));
    ymax = interval(0, height-1).clamp((int)std::ceil(ymax));

    for (int y = ymin; y <= ymax; y++){ //height
        for (int x = xmin; x <= xmax; x++){ //width
            if (insideTriangle(x + 0.5, y + 0.5, t.vertex)) {// pixel center = (x+0.5, y+0.5)

                if (x == 321 && y == 699 - 397)
                    printf("aaa\n");

                auto barycentric = computeBarycentric(x + 0.5, y + 0.5, t.vertex);
                auto correct = 1.0 / (barycentric[0] / vertPayload.v_viewspace[0].z() + barycentric[1] / vertPayload.v_viewspace[1].z() + barycentric[2] / vertPayload.v_viewspace[2].z());
                barycentric[0] = barycentric[0] / vertPayload.v_viewspace[0].z() * correct;
                barycentric[1] = barycentric[1] / vertPayload.v_viewspace[1].z() * correct;
                barycentric[2] = barycentric[2] / vertPayload.v_viewspace[2].z() * correct;

                auto z_interpolate = barycentric[0] * t[0][2] + barycentric[1] * t[1][2] + barycentric[2] * t[2][2];
                auto vt_interpolate = barycentricInterpolate(barycentric, t.vt);
                auto col_interpolate = barycentricInterpolate(barycentric, t.col);

                auto vn_viewspace_interpolate = barycentricInterpolate(barycentric, t.vn);
                vn_viewspace_interpolate = normalize(vn_viewspace_interpolate);
                auto v_viewspace_interpolate = barycentricInterpolate(barycentric, vertPayload.v_viewspace);

                inFragShaderPayload fragPayload = { v_viewspace_interpolate, vt_interpolate, vn_viewspace_interpolate, col_interpolate, t.getTexture() };

                auto triColor = fragShader.processPayload(fragPayload);

                if (0 <= z_interpolate && z_interpolate < depth_buffer[y * width + x]) {
                    depth_buffer[y * width + x] = z_interpolate;
                    setPixel(x, y, triColor);
                }
                
            }
        }
    }
}