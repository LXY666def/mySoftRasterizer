#pragma once

#include"global.hpp"
#include"texture.hpp"

class Triangle {
private:
	shared_ptr<Texture> texture = nullptr;
public:
	vec3 vertex[3];
	point2 vt[3];
	vec3 vn[3];
	color col[3];

	Triangle();
	Triangle(const vec3*, const vec3*, const vec3*, const color*, shared_ptr<Texture>);
	void setVertex(int index, const vec3& p);
	void setTexture(shared_ptr<Texture>);
	shared_ptr<Texture> getTexture()const;

	vec3& operator[](int index);
	vec3 operator[](int index)const;
};

Triangle::Triangle() {
	vertex[0] = vec3(0, 0, 0);
	vertex[1] = vec3(0, 0, 0);
	vertex[2] = vec3(0, 0, 0);
}
Triangle::Triangle(const vec3* _v, const vec3* _vt, const vec3* _vn, const color* _col = nullptr, shared_ptr<Texture> _texture = nullptr) {
	if (_v != nullptr)	    for (int i = 0; i < 3; i++)	vertex[i] = _v[i];
	if (_vt != nullptr)	    for (int i = 0; i < 3; i++)	vt[i] = _vt[i];
	if (_vn != nullptr)		for (int i = 0; i < 3; i++)	vn[i] = _vn[i];
	if (_col != nullptr)	for (int i = 0; i < 3; i++)	col[i] = _col[i];
	texture = _texture;
}
void Triangle::setVertex(int index, const vec3& p) {
	vertex[index] = p;
}
void Triangle::setTexture(shared_ptr<Texture> _texture) {
	texture = _texture;
}
shared_ptr<Texture> Triangle::getTexture()const {
	return texture;
}
vec3& Triangle::operator[](int index) {
	return vertex[index];
}
vec3 Triangle::operator[](int index)const {
	return vertex[index];
}