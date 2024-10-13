#pragma once

#include"matrix.hpp"
#include"onb.hpp"

#include"string"

constexpr int SR_CAMERA_CREATE_INFO = 1;
constexpr int SR_DOT_LIGHT_CREATE_INFO = 2;
constexpr int SR_PLANE_LIGHT_CREATE_INFO = 3;

typedef struct {
	int type;
	onb cam_coord;	// left-handed, 'cause g = -Z
	point3 e;	//eye_pos
	float near;
	float far;
	float fov;
	float aspect_ratio;
	int width;
	int height;
	float w_h_ratio;

	color intensity;
	float radius;
}CameraInfo;

class Camera {
private:
	CameraInfo Info;
	float* depthImage = nullptr;

	float changeZRange;

public:
	float inv_near_t;
	float inv_near_r;

	Camera(CameraInfo& _Info):Info(_Info) {
		_Info.height = getHeight();
		Info.height = _Info.height;
		inv_near_t = 1.0 / (fabs(Info.near) * tan(deg2rad(Info.fov / 2)));
		inv_near_r = inv_near_t / Info.aspect_ratio;
		changeZRange = 255.999 / (Info.far - Info.near);
	}

	matrix getViewMatrix()const;
	matrix getProjectionMatrix()const;
	int getWidth()const;
	int getHeight()const;
	float getChangeZRange()const;
	float getZNear()const { return Info.near; }
	float getZFar()const { return Info.far; }
	void claimDepthImage(float* data) { depthImage = data; }
	vec3 getLightPos()const { return Info.e; }
	color getLightIntensity()const { return Info.intensity; }
	void toCamSpace(const Camera& cam);
	float is_shadow(const vec3& dir) const;
	float queryDepth(int, int)const;
	onb getOnb()const;
};

using Light = Camera;

inline matrix Camera::getViewMatrix()const {
	matrix view_translation = Identity();
	view_translation[0][3] = -Info.e[0];
	view_translation[1][3] = -Info.e[1];
	view_translation[2][3] = -Info.e[2];

	matrix view_rotation = Identity();
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			view_rotation[i][j] = Info.cam_coord[i][j]*(i==2?-1:1);//g = -Z
		}
	}
	return view_rotation * view_translation;
}
inline matrix Camera::getProjectionMatrix()const {
	auto zNear = -Info.near;
	auto zFar = -Info.far;

	matrix persp2ortho;
	persp2ortho[0][0] = persp2ortho[1][1] = zNear;
	persp2ortho[2][2] = zFar + zNear;
	persp2ortho[2][3] = -zFar * zNear;
	persp2ortho[3][2] = 1;

	matrix ortho_translation = Identity();
	ortho_translation[0][3] = 0;	// -(l + r) / 2;
	ortho_translation[1][3] = 0;	// -(b + t) / 2;
	ortho_translation[2][3] = -(zNear + zFar) / 2;

	matrix ortho_scaling = Identity();
	ortho_scaling[0][0] = inv_near_r;
	ortho_scaling[1][1] = inv_near_t;
	ortho_scaling[2][2] = 2 / fabs(zNear - zFar);

	matrix ortho = ortho_scaling * ortho_translation;

	matrix projection = ortho * persp2ortho;
	return projection;

}
int Camera::getWidth()const {
	return Info.width;
}
int Camera::getHeight()const {
	return static_cast<int>(Info.width / Info.w_h_ratio);
}
float Camera::getChangeZRange()const {
	return changeZRange;
}
onb Camera::getOnb()const {
	return Info.cam_coord;
}
void Camera::toCamSpace(const Camera& cam) {
	matrix view_rotation = Identity();
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			view_rotation[i][j] = cam.Info.cam_coord[i][j] * (i == 2 ? -1 : 1);//g = -Z
		}
	}
	Info.cam_coord.axis[0] = view_rotation * Info.cam_coord.axis[0];
	Info.cam_coord.axis[1] = view_rotation * Info.cam_coord.axis[1];
	Info.cam_coord.axis[2] = view_rotation * Info.cam_coord.axis[2];

	Info.e = cam.getViewMatrix() * Info.e;
}
float Camera::queryDepth(int x, int y)const {
	return depthImage[y * Info.width + x];
}
float Camera::is_shadow(const vec3& dir) const {
	if (depthImage == nullptr)	return 0;	// not shadow

	float bias = 0.6;
	// to light space
	auto direction = normalize(dir);
	auto dir_viewspace = Info.cam_coord.world2local(direction);
	// dir_viewspace intersect with near plane
	float t = fabs(Info.near / dir_viewspace[2]);
	auto intersection = dir_viewspace * t;
	float u = 0.5 * intersection.x() * inv_near_r + 0.5;
	float v = 0.5 * intersection.y() * inv_near_t + 0.5;
	if (u < 0 || u>1 || v < 0 || v>1)	return 1;	// all shadow

	auto x = static_cast<int>(u * Info.width);
	auto y = static_cast<int>(v * Info.height);

	// blocker search, get average blocker depth
	auto zDepth = dotProduct(dir, Info.cam_coord[2]);
	auto query_radius = static_cast<int>(Info.radius * (1.0 - Info.near / zDepth));
	auto dist = (zDepth - Info.near) * changeZRange;
	float result = 0.0;
	int cnt = 0;
	for (int j = y - query_radius; j <= y + query_radius; j++) {
		for (int i = x - query_radius; i <= x + query_radius; i++) {
			auto blockerDepth = queryDepth(i, j);
			if (dist < blockerDepth + bias) continue;
			cnt++;
			result += blockerDepth;
		}
	}
	if (cnt == 0)	return 0;
	auto avg_blockerDepth = result / cnt;

	//PCF
	auto w_penumbra = (dist / avg_blockerDepth - 1.0) * 2 * Info.radius;
	auto filter_size = static_cast<int>(w_penumbra);
	if (filter_size > 20)	filter_size = 20;
	result = 0.0;
	for (int j = y - filter_size; j <= y + filter_size; j++) {
		for (int i = x - filter_size; i <= x + filter_size; i++) {
			if (dist > queryDepth(i, j) + bias)	result += 1.0f;
		}
	}
	
	return interval(0, 1).clamp(result / pow(1 + 2 * filter_size, 2));
}