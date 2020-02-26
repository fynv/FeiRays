#pragma once

#include "PathTracer.h"
#include "ColoredUnitSphere.h"
#include "ColoredIndexedTriangleList.h"
#include "TexturedUnitSphere.h"
#include "WavefrontIndexedTriangleList.h"
#include "texture_map.h"

#include <vector>

class Scene
{
public:
	Scene(int width, int height);
	~Scene();

	const Image& image() const { return m_Image; }
	void set_gradient_sky(const glm::vec3& color0, const glm::vec3& color1);
	void set_textured_sky(const char* fn_tex, bool srgb);
	void set_textured_sky(const char* fn_tex, bool srgb, float angle, const glm::vec3& v);
	void add_colored_sphere(const glm::mat4x4& model, const Material& material);
	void add_colored_cube(const glm::mat4x4& model, const Material& material);
	void add_textured_sphere(const glm::mat4x4& model, const char* fn_tex, bool srgb, const glm::vec3& color);
	void add_wavefront_object(const glm::mat4x4& model, const char* path, const char* fn);
	void add_sphere_light(const glm::vec3& center, float r, const glm::vec3& color);
	void add_sunlight(glm::vec3 direction, float radian, glm::vec3 color)
	{
		m_pt.add_sunlight(direction, radian, color);
	}
	void set_camera(glm::vec3 lookfrom, glm::vec3 lookat, glm::vec3 vup, float vfov, float aperture, float focus_dist)
	{
		m_pt.set_camera(lookfrom, lookat, vup, vfov, aperture, focus_dist);
	}
	void trace(int num_iter, int interval) const
	{
		m_pt.trace(num_iter, interval);
	}

private:
	Image m_Image;
	PathTracer m_pt;
	TextureMap m_tex_map;
	Sky* m_sky;
	std::vector<Geometry*> m_geos;
};


