#pragma once

#include<string.h>
#include<iostream>
#include<vector>
#include"vec3.hpp"
#include<stdio.h>

enum f_sscanf_format {
    __V = 1,
    __V_VT = 2,
    __V_VT_VN = 3,
    __V__VN = 4
};
bool findFormat(enum f_sscanf_format&, const char*);

class ObjLoader {
private:
    size_t triangle_number;
    std::vector<vec3> vertex_buffer;
    std::vector<vec3> vn_buffer;
    std::vector<vec3> vt_buffer;
    std::vector<std::vector<int>> v_index_buffer;
    std::vector<std::vector<int>> vt_index_buffer;
    std::vector<std::vector<int>> vn_index_buffer;
public:
    ObjLoader(const char* filename);
    ~ObjLoader();
    std::vector<Triangle*> getTriangleList(shared_ptr<Texture>)const;
    int nverts();
    vec3 vertexBuffer(int i);
    std::vector<int> indexBuffer(int idx);
};

ObjLoader::ObjLoader(const char* filename){
    FILE* in;
    if (fopen_s(&in, filename, "r") != 0) {
        std::cerr << "Failed to open obj file!";
    }
    char line[256] = {};
    bool isFormatFinded = false;
    enum f_sscanf_format format;
    while (fgets(line, sizeof(line), in)) {
        if (line[0] == 'v') {
            if (line[1] == ' ') {
                float x, y, z;
                sscanf_s(line, "v %f %f %f", &x, &y, &z);
                vertex_buffer.push_back(vec3(x, y, z, 1.0));
            }
            else if (line[1] == 't') {
                float x, y;
                sscanf_s(line, "vt %f %f", &x, &y);
                vt_buffer.push_back(vec3(x, y, 0.0));
            }
            else {
                float x, y, z;
                sscanf_s(line, "vn %f %f %f", &x, &y, &z);
                vn_buffer.push_back(vec3(x, y, z, 0.0));
            }
        }
        else if (line[0] == 'f') {
            if (!isFormatFinded) {
                if (!findFormat(format, line)) {
                    std::cerr << "failed to load obj file, find format wrongly!\n";
                }
                isFormatFinded = true;
            }
            std::vector<int> v_indBuf(3), vt_indBuf(3), vn_indBuf(3);
            switch (format) {
            case __V:
                sscanf_s(line, "f %d %d %d", &v_indBuf[0], &v_indBuf[1], &v_indBuf[2]);
                break;
            case __V_VT:
                sscanf_s(line, "f %d/%d %d/%d %d/%d", &v_indBuf[0], &vt_indBuf[0], &v_indBuf[1], &vt_indBuf[1], &v_indBuf[2], &vt_indBuf[2]);
                break;
            case __V_VT_VN:
                sscanf_s(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &v_indBuf[0], &vt_indBuf[0], &vn_indBuf[0],
                    &v_indBuf[1], &vt_indBuf[1], &vn_indBuf[1],
                    &v_indBuf[2], &vt_indBuf[2], &vn_indBuf[2]);
                break;
            case __V__VN:
                sscanf_s(line, "f %d//%d %d//%d %d//%d", &v_indBuf[0], &vn_indBuf[0], &v_indBuf[1], &vn_indBuf[1], &v_indBuf[2], &vn_indBuf[2]);
                break;
            default:
                break;
            }
            for (int i = 0; i < 3; i++) {
                v_indBuf[i]--; vt_indBuf[i]--; vn_indBuf[i]--;
            }
            v_index_buffer.push_back(v_indBuf);
            vt_index_buffer.push_back(vt_indBuf);
            vn_index_buffer.push_back(vn_indBuf);
        }//if
    }//while
    triangle_number = v_index_buffer.size();
    fclose(in);
}

std::vector<Triangle*> ObjLoader::getTriangleList(shared_ptr<Texture> _texture = nullptr)const {

    std::vector<Triangle*> list;
    for (int i = 0; i < triangle_number; i++) {
        vec3 v[3] = { vertex_buffer[v_index_buffer[i][0]], vertex_buffer[v_index_buffer[i][1]], vertex_buffer[v_index_buffer[i][2]]};
        vec3 vt[3] = { vec3(0),vec3(0),vec3(0) };
        if (!vt_buffer.empty()) {
            vt[0] = vt_buffer[vt_index_buffer[i][0]];
            vt[1] = vt_buffer[vt_index_buffer[i][1]];
            vt[2] = vt_buffer[vt_index_buffer[i][2]];
        }
        vec3 vn[3] = { vn_buffer[vn_index_buffer[i][0]], vn_buffer[vn_index_buffer[i][1]], vn_buffer[vn_index_buffer[i][2]] };

        Triangle* t = new Triangle(v, vt, vn);
        t->setTexture(_texture);
        //std::cout << p1.x() << ' ' << p1.y() << ' ' << p2.x() << ' ' << p2.y() << ' ' << p3.x() << ' ' << p3.y() << '\n';
        list.push_back(t);
    }

    return list;
}

ObjLoader::~ObjLoader() {
}

int ObjLoader::nverts() {
    return (int)vertex_buffer.size();
}


std::vector<int> ObjLoader::indexBuffer(int idx) {
    return v_index_buffer[idx];
}

vec3 ObjLoader::vertexBuffer(int i) {
    return vertex_buffer[i];
}

bool findFormat(enum f_sscanf_format& format, const char* line) {
    auto first = strchr(line, '/');
    if (first == NULL) {
        format = __V;
        return true;
    }
    auto second = strchr(first + 1, '/');
    auto third = strchr(second + 1, '/');
    auto fourth = strchr(third + 1, '/');
    if (fourth == NULL) {
        format = __V_VT;
        return true;
    }
    if (second - first == 1) {
        format = __V__VN;
        return true;
    }
    else {
        format = __V_VT_VN;
        return true;
    }
    return false;
}