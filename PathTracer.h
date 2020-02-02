#pragma once

#include <glm.hpp>
#include <string>
#include <vector>
#include <unordered_map>

class BaseLevelAS;
class DeviceBuffer;

struct GeoCls
{
	std::string name;
	size_t size_view;
	uint32_t binding_view;
	const char* fn_intersection = nullptr;
	const char* fn_closesthit = nullptr;
};

class Geometry
{
public:
	const glm::mat4x4& model() const { return m_model; }
	const glm::mat4x4& norm() const { return m_norm_mat; }
	const BaseLevelAS& get_blas() const { return *m_blas; }

	Geometry(const glm::mat4x4& model);
	virtual ~Geometry();

	virtual GeoCls cls() const = 0;
	virtual void get_view(char* view_buf) const = 0;

protected:
	glm::mat4x4 m_model;
	glm::mat4x4 m_norm_mat;
	BaseLevelAS* m_blas;

};


class Image
{
public:
	DeviceBuffer* data() const { return m_data; }
	int width() const { return m_width; }
	int height() const { return m_height; }

	Image(int width, int height, float* hdata = nullptr);
	~Image();

	void clear();
	void to_host(void *hdata) const;

private:
	DeviceBuffer* m_data;
	int m_width;
	int m_height;

};

struct GeoList
{
	GeoCls cls;
	std::vector<Geometry*> list;
};

struct RayTrace;

class PathTracer
{
public:
	PathTracer();
	~PathTracer();
	
	void set_target(Image* target) { m_target = target;  }
	void add_geometry(Geometry* geo);
	void set_camera(glm::vec3 lookfrom, glm::vec3 lookat, glm::vec3 vup, float vfov, float focus_dist = 1.0f);

	void trace(int num_iter = 100) const;

private:
	Image* m_target;
	std::unordered_map<std::string, GeoList> m_geo_lists;

	// camera
	glm::vec3 m_lookfrom;
	glm::vec3 m_lookat;
	glm::vec3 m_vup;
	float m_vfov;
	float m_focus_dist;

	void _tlas_create(RayTrace& rt) const;
	void _args_create(RayTrace& rt) const;
	void _rt_pipeline_create(RayTrace& rt) const;
	void _comp_pipeline_create(RayTrace& rt) const;
	void _calc_raygen(RayTrace& rt) const;
	void _rand_init_cpu(RayTrace& rt) const;
	void _rand_init_cuda(RayTrace& rt) const;

	void _rt_clean(RayTrace& rt) const;
	
};
