#pragma once

#include"triangle.hpp"
#include"matrix.hpp"
#include"camera.hpp"

typedef struct {
	Triangle triangle;
	point3 v_viewspace[3];
}outVertShaderPayload;

class VERT_SHADER {
public:
	VERT_SHADER(const Camera& cam, const matrix& _model) {
		model = _model;
		view = cam.getViewMatrix();
		projection = cam.getProjectionMatrix();

		viewport = Identity();
		viewport[0][0] = viewport[0][3] = static_cast<float>(cam.getWidth()) / 2;
		viewport[1][1] = viewport[1][3] = static_cast<float>(cam.getHeight()) / 2;
		viewport[2][2] = viewport[3][3] = 1.0;

		mv = view * model;
		mvp = projection * mv;
		vn_trans = Transpose(Inversed(view * model));
	}
	virtual ~VERT_SHADER() = default;
	virtual outVertShaderPayload processVertData(const Triangle*)const = 0;
	virtual void setModelMatrix(const matrix& m) { model = m; }
	virtual matrix getViewMatrix() const { return view; }
protected:
	matrix model;
	matrix view;
	matrix projection;
	matrix viewport;
	matrix mvp;
	matrix mv;
	matrix vn_trans;
};

class VERT_SHADER_standard : public VERT_SHADER {
public:
	VERT_SHADER_standard(const Camera& cam, const matrix& _model):VERT_SHADER(cam, _model), zNear(cam.getZNear()), changeZRange(cam.getChangeZRange()) {}
	outVertShaderPayload processVertData(const Triangle* triangle) const override {
		outVertShaderPayload payload;
		vec3 vert[3] , _vn_viewspace[3], _v_viewspace[3];
		interval vert_z(0, 0), viewspace_z(0, 0);
		for (int i = 0; i < 3; i++) {
			vert[i] = mvp * (*triangle)[i];       // mvp transformation
			vert[i] /= vert[i][3];          // homogeneous division
			vert[i] = viewport * vert[i];   // viewport transformation
			_vn_viewspace[i] = normalize(vn_trans * (*triangle).vn[i]);  // vn with model view transformation

			payload.v_viewspace[i] = view * model * (*triangle)[i];
			vert[i][2] = -(payload.v_viewspace[i][2] + zNear) * changeZRange;

			viewspace_z.noteRange(payload.v_viewspace[i][2]);
		}
		payload.triangle = Triangle(vert, triangle->vt, _vn_viewspace, nullptr, triangle->getTexture());
		//viewspace_z.print();
		return payload;
	}
private:
	float zNear;
	float changeZRange;
};

class VERT_SHADER_gouraud : public VERT_SHADER {
public:
	VERT_SHADER_gouraud(const Camera& cam, const matrix& _model) :VERT_SHADER(cam, _model) {}
	outVertShaderPayload processVertData(const Triangle* triangle) const override {
		// light in camera space
		point3 light_pos(40, 40, 15);
		color light_intensity(WHITE * 8000);

		outVertShaderPayload payload;
		color col[3], _vn_viewspace[3],vert[3];
		// lambertian diffuse
		color kd(0.85,0.40,0.0);
		for (int i = 0; i < 3; i++) {
			vert[i] = mvp * (*triangle)[i];       // mvp transformation
			vert[i] /= vert[i][3];          // homogeneous division
			vert[i] = viewport * vert[i];   // viewport transformation

			payload.v_viewspace[i] = view * model * (*triangle)[i];
			_vn_viewspace[i] = normalize(vn_trans * (*triangle).vn[i]);

			auto dist = (light_pos - payload.v_viewspace[i]);
			auto intensity = light_intensity / dist.norm2();
			col[i] = kd * intensity * std::max(0.0f, dotProduct(normalize(dist), _vn_viewspace[i]));
		}

		payload.triangle = Triangle(vert, nullptr, nullptr, col, nullptr);

		return payload;
	}
};

class VERT_SHADER_TBN_visualization : public VERT_SHADER {
public:
	VERT_SHADER_TBN_visualization(const Camera& cam, const matrix& _model) :VERT_SHADER(cam, _model) {}
	outVertShaderPayload processVertData(const Triangle* triangle) const override {
		outVertShaderPayload payload;
		onb TBN;

		auto v01 = triangle->vertex[1] - triangle->vertex[0];
		auto v02 = triangle->vertex[2] - triangle->vertex[0];
		TBN[2] = normalize(crossProduct(v01, v02));

		// TBN
		auto u1 = triangle->vt[1][0] - triangle->vt[0][0];
		auto u2 = triangle->vt[2][0] - triangle->vt[0][0];
		if (u1 == 0) {
			TBN[0] = normalize(crossProduct(TBN.w(), v01)) * (u2 ? 1 : -1);
		}else {
			auto t = u2 / u1;
			TBN[0] = normalize(crossProduct(TBN.w(), t * v01 - v02)) * (u1 ? 1 : -1);
		}
		TBN[1] = crossProduct(TBN.w(), TBN.u());

		vec3 vert[3], _vn_viewspace[3], _v_viewspace[3];
		for (int i = 0; i < 3; i++) {
			vert[i] = mvp * (*triangle)[i];       // mvp transformation
			vert[i] /= vert[i][3];          // homogeneous division
			vert[i] = viewport * vert[i];   // viewport transformation
			_vn_viewspace[i] = TBN.world2local(triangle->vn[i]);  // vn with model view transformation
		}
		payload.triangle = Triangle(vert, triangle->vt, _vn_viewspace, nullptr, nullptr);

		return payload;
	}
};
class VERT_SHADER_depth : public VERT_SHADER {
public:
	VERT_SHADER_depth(const Camera& cam, const matrix& _model) :VERT_SHADER(cam, _model), zNear(cam.getZNear()), zFar(cam.getZFar()) {}
	outVertShaderPayload processVertData(const Triangle* triangle) const override {
		outVertShaderPayload payload;
		vec3 vert[3], _vn_viewspace[3], _v_viewspace[3];
		float changeZRange = -255.999 / (zFar - zNear);
		interval zz(-20, -20);
		for (int i = 0; i < 3; i++) {
			vert[i] = mvp * (*triangle)[i];       // mvp transformation
			vert[i] /= vert[i][3];          // homogeneous division
			vert[i] = viewport * vert[i];   // viewport transformation

			payload.v_viewspace[i] = mv * (*triangle)[i];
			vert[i][2] = (payload.v_viewspace[i][2] + zNear) * changeZRange;
			zz.noteRange(payload.v_viewspace[i][2]);
		}
		payload.triangle = Triangle(vert, triangle->vt, _vn_viewspace, nullptr, triangle->getTexture());
		zz.print();
		return payload;
	}
private:
	float zNear;
	float zFar;
};