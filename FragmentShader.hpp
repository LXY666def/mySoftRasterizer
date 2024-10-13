#pragma once

#include"global.hpp"
#include"color.hpp"
#include"texture.hpp"

typedef struct {
	point3 v_viewspace;
	point2 vt;
	vec3 vn_viewspace;
	color col;
	shared_ptr<Texture> texture;
}inFragShaderPayload;

class FRAG_SHADER {

public:
	virtual ~FRAG_SHADER() = default;
	virtual color processPayload(const inFragShaderPayload& inPayload) const = 0;
};
// return color based on the interpolated normal
class FRAG_SHADER_normal : public FRAG_SHADER {
public:
	color processPayload(const inFragShaderPayload& inPayload)const override {
		auto result = inPayload.vn_viewspace;
		return color((result[0] + 1.0) / 2, (result[1] + 1.0) / 2, (result[2] + 1.0) / 2);
	}
};
class FRAG_SHADER_phong : public FRAG_SHADER {
private:
	Light light;
public:
	FRAG_SHADER_phong(const Light& _light):light(_light){}

	color processPayload(const inFragShaderPayload& inPayload)const override {
		auto u = inPayload.vt[0];
		auto v = inPayload.vt[1];
		auto p = inPayload.v_viewspace;
		auto normal = inPayload.vn_viewspace;
		point3 eye_pos(0);
		auto intensity = light.getLightIntensity();

		// light in camera space
		color ka(0.01);
		color amb_light_intensity(10);
		color ambiant = ka * amb_light_intensity;

		// shadow test
		auto to_light = light.getLightPos() - p;
		auto coe_shadow = light.is_shadow(-to_light);

		to_light = normalize(to_light);
		auto inv_dist2 = 1.0 / (light.getLightPos() - p).norm2();
		auto to_eye = normalize(eye_pos - p);
		auto bivector = normalize(to_eye + to_light);

		
		color kd = inPayload.texture->value(u,v,p);
		color ks(0.05);

		color result(0);
		// diffuse
		result += kd * intensity * inv_dist2 * fmax(0.0, dotProduct(normal, to_light));
		// specular
		result += ks * intensity * inv_dist2 * std::pow(std::max(0.0f, dotProduct(bivector, normal)), 100);
		// ambient
		result += ambiant;

		return interpolate_vec3(pow(coe_shadow, 4), ambiant, result);
	}
};

class FRAG_SHADER_texture_image : public FRAG_SHADER {
	color processPayload(const inFragShaderPayload& inPayload)const override {
		auto u = inPayload.vt[0];
		auto v = inPayload.vt[1];
		auto triColor = inPayload.texture->value(u, v, inPayload.v_viewspace);
		return triColor;
	}
};
// interpolate vertexs color directly
class FRAG_SHADER_vertex_color : public FRAG_SHADER {
	color processPayload(const inFragShaderPayload& inPayload)const override {
		return inPayload.col;
	}
};
// interpolate vertexs color directly
class FRAG_SHADER_freestyle : public FRAG_SHADER {
	color processPayload(const inFragShaderPayload& inPayload)const override {
		color kd(0.85, 0.40, 0.0);
		color result;
		if (inPayload.col[0] > .85) result = 1 * kd;
		else if (inPayload.col[0] > .60) result = .85 * kd;
		else if (inPayload.col[0] > .45) result = .65 * kd;
		else if (inPayload.col[0] > .30) result = .50 * kd;
		else if (inPayload.col[0] > .15) result = .35 * kd;
		else result = 0.20 * kd;
		return result;
	}
};
class FRAG_SHADER_depth : public FRAG_SHADER {
	color processPayload(const inFragShaderPayload& inPayload)const override {
		return color(1.0-inPayload.v_viewspace[2]/255.999);
	}
};