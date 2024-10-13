#pragma once

#include"my_image.hpp"

class HDRI {
private:
	my_image image;
	float rotate;
public:
	HDRI(const char* filename, float rotate);

	color queryColor(const vec3& v)const;
};

HDRI::HDRI(const char* filename, float rotate = 0) :image(filename), rotate(rotate / 360) {}

color HDRI::queryColor(const vec3& vec)const {
	// x = sin_theta * cos_phi
	// y = sin_theta * sin_phi
	// z = cos_theta
	auto theta = acos(-vec.y());
	auto phi = atan2(-vec.z(), vec.x()) + pi;
	auto v = theta / pi;
	auto u = phi / pi * 0.5;
	u -= rotate;
	if (u < 0)	u += 1;

	auto col = image.texture_data(1-u, v);
	color result(col[0] / 255.0, col[1] / 255.0, col[2] / 255.0);

	return result;
}
