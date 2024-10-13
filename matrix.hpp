#pragma once

#include"vec3.hpp"

class matrix {
public:
	float ma[4][4] = {};

	float* operator[](int );
};

inline matrix operator/(const matrix& m, float d) {
    if (fabs(d) < 1e-8) {
        std::cerr << "inline matrix operator/(const matrix& m, float d): ERROR d=0!\n";
        exit(1);
    }
    float inv_d = 1.0 / d;
    matrix result;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++)
            result[i][j] = m.ma[i][j] * inv_d;
    }
    return result;
}

float Det4(matrix& m) {
    double det = 0;
    det = m[0][0] * (
        m[1][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
        m[1][2] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) +
        m[1][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1])
    ) - m[0][1] * (
        m[1][0] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
        m[1][2] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) +
        m[1][3] * (m[2][0] * m[3][2] - m[2][2] * m[3][0])
    ) + m[0][2] * (
        m[1][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) -
        m[1][1] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) +
        m[1][3] * (m[2][0] * m[3][1] - m[2][1] * m[3][0])
    ) - m[0][3] * (
        m[1][0] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]) -
        m[1][1] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]) +
        m[1][2] * (m[2][0] * m[3][1] - m[2][1] * m[3][0])
    );

    return det;
}
float Det3(matrix m) {
    return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
        m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
        m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
}
inline matrix Transpose(const matrix& ma) {
    matrix result;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)	result[i][j] = ma.ma[j][i];
    return result;
}
matrix Company(matrix m) {
    matrix cofactor;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            matrix submatrix;

            int subi = 0;
            for (int row = 0; row < 4; row++) {
                if (row == i) continue;

                int subj = 0;
                for (int col = 0; col < 4; col++) {
                    if (col == j) continue;

                    submatrix[subi][subj] = m[row][col];
                    subj++;
                }
                subi++;
            }
            double sign = ((i + j) % 2 == 0) ? 1 : -1;
            cofactor[i][j] = sign * Det3(submatrix);
        }
    }

    return Transpose(cofactor);
}
matrix Inversed(matrix m) {
    return Company(m) / Det4(m);
}
float* matrix::operator[](int i) {
	return ma[i];
}
inline matrix Identity() {
	matrix m;
	m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1;
	return m;
}
inline matrix operator+(const matrix& m1, const matrix& m2) {
	matrix result;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)	result[i][j] = m1.ma[i][j] + m2.ma[i][j];
	return result;
}
inline matrix operator*(const matrix& m1, const matrix& m2) {
	matrix result;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			for (int k = 0; k < 4; k++)	result[i][j] += m1.ma[i][k] * m2.ma[k][j];

	return result;
}
inline vec3 operator*(const matrix& m1, const vec3& v) {
	vec3 result(0.0);
	for (int i = 0; i < 4; i++)
		for (int k = 0; k < 4; k++)	result[i] += m1.ma[i][k] * v[k];
	return result;
}
inline matrix getRotateXMatrix(float alpha) {
    alpha = deg2rad(alpha);
    matrix rotate_x = Identity();
    rotate_x[1][1] = rotate_x[2][2] = cos(alpha);
    rotate_x[2][1] = sin(alpha);
    rotate_x[1][2] = -sin(alpha);
    return rotate_x;
}
inline matrix getRotateYMatrix(float beta) {
    beta = deg2rad(beta);
    matrix rotate_y = Identity();
    rotate_y[0][0] = rotate_y[2][2] = cos(beta);
    rotate_y[2][0] = -sin(beta);
    rotate_y[0][2] = sin(beta);
    return rotate_y;
}
inline matrix getRotateZMatrix(float gamma) {
    gamma = deg2rad(gamma);
    matrix rotate_z = Identity();
    rotate_z[0][0] = rotate_z[1][1] = cos(gamma);
    rotate_z[1][0] = sin(gamma);
    rotate_z[0][1] = -sin(gamma);
    return rotate_z;
}
inline matrix getModelMatrix(const vec3& scale, float alpha, float beta, float gamma, const vec3& translation) {
	matrix scale_model = Identity();
	for (int i = 0; i < 3; i++)	scale_model[i][i] = scale[i];

	matrix model = getRotateZMatrix(gamma) * getRotateYMatrix(beta) * getRotateXMatrix(alpha) * scale_model * Identity();
	for (int i = 0; i < 3; i++)	model[i][3] = translation[i];

	return model;
}
inline matrix getModelMatrix(float alpha, float beta, float gamma, const vec3& scale, const vec3& translation) {
    matrix scale_model = Identity();
    for (int i = 0; i < 3; i++)	scale_model[i][i] = scale[i];

    matrix model = scale_model * getRotateZMatrix(gamma) * getRotateYMatrix(beta) * getRotateXMatrix(alpha) * Identity();
    for (int i = 0; i < 3; i++)	model[i][3] = translation[i];

    return model;
}