#include "api.h"
#include "Scene.h"
#define _USE_MATH_DEFINES
#include <math.h>

void* n_scene_create(int width, int height)
{
	Scene* scene = new Scene(width, height);
	return scene;
}

void n_scene_destroy(void* cptr)
{
	Scene* scene = (Scene*)cptr;
	delete scene;
}

int n_scene_width(void* cptr)
{
	Scene* scene = (Scene*)cptr;
	return scene->image().width();
}

int n_scene_height(void* cptr)
{
	Scene* scene = (Scene*)cptr;
	return scene->image().height();
}

void n_scene_get_image(void* cptr, float boost, void* data_out)
{
	Scene* scene = (Scene*)cptr;
	const Image& image = scene->image();
	int width = image.width();
	int height = image.height();
	image.to_host_srgb((unsigned char*)data_out, boost);
}

void n_scene_set_gradient_sky(void* ptr_scene, void* ptr_color0, void* ptr_color1)
{
	Scene* scene = (Scene*)ptr_scene;
	glm::vec3* color0 = (glm::vec3*)ptr_color0;
	glm::vec3* color1 = (glm::vec3*)ptr_color1;
	scene->set_gradient_sky(*color0, *color1);
}

void n_scene_set_textured_sky(void* ptr_scene, const char* fn_tex, unsigned tex_srgb, float angle, void* ptr_vec)
{
	Scene* scene = (Scene*)ptr_scene;
	if (ptr_vec != nullptr && angle != 0.0f)
	{
		float rad = angle / 180.0f * (float)M_PI;
		glm::vec3* v = (glm::vec3*)ptr_vec;
		scene->set_textured_sky(fn_tex, tex_srgb!=0, rad, *v);
	}
	else
	{
		scene->set_textured_sky(fn_tex, tex_srgb != 0);
	}
}

void n_scene_add_colored_sphere(void* ptr_scene, void* ptr_trans, int mat_type, void* ptr_mat_color, float mat_fuzz, float mat_ref_idx, float mat_density)
{
	Scene* scene = (Scene*)ptr_scene;
	glm::mat4x4* trans = (glm::mat4x4*)ptr_trans;
	Material mat;
	mat.type = (MaterialType)mat_type;
	if (ptr_mat_color != nullptr) mat.color = *(glm::vec3*)ptr_mat_color;
	mat.fuzz = mat_fuzz;
	mat.ref_idx = mat_ref_idx;
	mat.density = mat_density;
	scene->add_colored_sphere(*trans, mat);
}

void n_scene_add_colored_cube(void* ptr_scene, void* ptr_trans, int mat_type, void* ptr_mat_color, float mat_fuzz, float mat_ref_idx, float mat_density)
{
	Scene* scene = (Scene*)ptr_scene;
	glm::mat4x4* trans = (glm::mat4x4*)ptr_trans;
	Material mat;
	mat.type = (MaterialType)mat_type;
	if (ptr_mat_color != nullptr) mat.color = *(glm::vec3*)ptr_mat_color;
	mat.fuzz = mat_fuzz;
	mat.ref_idx = mat_ref_idx;
	mat.density = mat_density;
	scene->add_colored_cube(*trans, mat);
}

void n_scene_add_textured_sphere(void* ptr_scene, void* ptr_trans, const char* fn_tex, unsigned tex_srgb, void* ptr_color)
{
	Scene* scene = (Scene*)ptr_scene;
	glm::mat4x4* trans = (glm::mat4x4*)ptr_trans;
	glm::vec3* color = (glm::vec3*)ptr_color;
	scene->add_textured_sphere(*trans, fn_tex, tex_srgb!=0, *color);
}

void n_scene_add_wavefront_object(void* ptr_scene, void* ptr_trans, const char* path, const char* fn)
{
	Scene* scene = (Scene*)ptr_scene;
	glm::mat4x4* trans = (glm::mat4x4*)ptr_trans;
	scene->add_wavefront_object(*trans, path, fn);
}

void n_scene_add_sphere_light(void* ptr_scene, void* ptr_center, float radius, void* ptr_color)
{
	Scene* scene = (Scene*)ptr_scene;
	glm::vec3* center = (glm::vec3*)ptr_center;
	glm::vec3* color = (glm::vec3*)ptr_color;
	scene->add_sphere_light(*center, radius, *color);
}

void n_scene_add_sunlight(void* ptr_scene, void* ptr_dir, float angle, void* ptr_color)
{
	Scene* scene = (Scene*)ptr_scene;
	glm::vec3* dir = (glm::vec3*)ptr_dir;
	float rad = angle / 180.0f * (float)M_PI;
	glm::vec3* color = (glm::vec3*)ptr_color;
	scene->add_sunlight(*dir, rad, *color);
}

void n_scene_set_camera(void* ptr_scene, void* ptr_lookfrom, void* ptr_lookat, void* ptr_vup, float vfov, float aperture, float focus_dist)
{
	Scene* scene = (Scene*)ptr_scene;
	glm::vec3* lookfrom = (glm::vec3*)ptr_lookfrom;
	glm::vec3* lookat = (glm::vec3*)ptr_lookat;
	glm::vec3* vup = (glm::vec3*)ptr_vup;
	scene->set_camera(*lookfrom, *lookat, *vup, vfov, aperture, focus_dist);
}

void n_scene_trace(void* ptr_scene, int num_iter, int interval)
{
	Scene* scene = (Scene*)ptr_scene;
	scene->trace(num_iter, interval);
}
