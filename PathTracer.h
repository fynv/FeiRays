#pragma once

#include <glm.hpp>
#include <string>
#include <vector>
#include <unordered_map>

class BaseLevelAS;
class DeviceBuffer;
class Texture;
class Cubemap;
class Sampler;

struct GeoCls
{
	std::string name;
	size_t size_view;
	uint32_t binding_view;
	const char* fn_intersection = nullptr;
	const char* fn_closesthit = nullptr;
};

enum Material
{
	lambertian,
	metal,
	dielectric
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
	virtual void get_view(void* view_buf) const = 0;

protected:
	glm::mat4x4 m_model;
	glm::mat4x4 m_norm_mat;
	BaseLevelAS* m_blas;
};

struct SkyCls
{
	size_t size_view;
	const char* fn_missing = nullptr;
};

// This is the default sky-box
class SkyBox
{
public:
	SkyBox();
	virtual ~SkyBox();

	virtual SkyCls cls() const;
	virtual void get_view(void* view_buf) const;
};

class RGBATexture
{
public:
	Texture* data() const { return m_data; }

	RGBATexture(int width, int height, void* data);
	~RGBATexture();	

private:
	Texture *m_data;	
};

class RGBACubemap
{
public:
	Cubemap* data() const { return m_data; }

	RGBACubemap(int width, int height, void* data);
	~RGBACubemap();

private:
	Cubemap* m_data;
};

class TexturedSkyBox : public SkyBox
{
public:
	TexturedSkyBox(int texId);
	virtual ~TexturedSkyBox();

	virtual SkyCls cls() const;
	virtual void get_view(void* view_buf) const;

	void rotate(float angle, const glm::vec3& v);
	void set_gamma(float gamma) { m_gamma = gamma;  }

private:
	int m_texId;
	glm::mat4x4 m_transform;
	float m_gamma;
};

class Image
{
public:
	DeviceBuffer* data() const { return m_data; }
	DeviceBuffer* rng_states(); 
	int width() const { return m_width; }
	int height() const { return m_height; }

	Image(int width, int height, float* hdata = nullptr);
	~Image();

	void clear();
	void to_host(void *hdata) const;

private:
	DeviceBuffer* m_data;
	DeviceBuffer* m_rng_states;
	int m_width;
	int m_height;

	void _rand_init_cpu() const;
	void _rand_init_cuda() const;
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
	void set_skybox(SkyBox* skybox) { m_current_sky_box = skybox; }
	void add_geometry(Geometry* geo);
	int add_texture(RGBATexture* tex);
	int add_cubemap(RGBACubemap* tex);
	void set_camera(glm::vec3 lookfrom, glm::vec3 lookat, glm::vec3 vup, float vfov, float aperture = 0.0f, float focus_dist = 1.0f);

	void trace(int num_iter = 100, int interval = -1) const;

private:
	Image* m_target;
	std::unordered_map<std::string, GeoList> m_geo_lists;
	std::vector<RGBATexture*> m_textures;
	std::vector<RGBACubemap*> m_cubemaps;
	Sampler* m_Sampler;
	SkyBox* m_current_sky_box;

	// camera
	glm::vec3 m_lookfrom;
	glm::vec3 m_lookat;
	glm::vec3 m_vup;
	float m_vfov;
	float m_aperture;
	float m_focus_dist;

	void _tlas_create(RayTrace& rt) const;
	void _args_create(RayTrace& rt) const;
	void _rt_pipeline_create(RayTrace& rt) const;
	void _comp_pipeline_create(RayTrace& rt) const;
	void _calc_raygen(RayTrace& rt) const;

	void _rt_clean(RayTrace& rt) const;
	
};
