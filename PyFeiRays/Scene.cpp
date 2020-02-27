#include "Scene.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

Scene::Scene(int width, int height) : m_Image(width, height), m_tex_map(&m_pt)
{
	m_pt.set_target(&m_Image);
	m_sky = nullptr;
}

Scene::~Scene()
{
	delete m_sky;
	for (size_t i = 0; i < m_geos.size(); i++)
		delete m_geos[i];
}

void Scene::set_gradient_sky(const glm::vec3& color0, const glm::vec3& color1)
{
	delete m_sky;
	m_sky = new GradientSky(color0, color1);
	m_pt.set_sky(m_sky);
}

void Scene::set_textured_sky(const char* fn_tex, bool srgb)
{
	int texId = m_tex_map.findTex(fn_tex, srgb);
	delete m_sky;
	m_sky = new TexturedSkyBox(texId);
	m_pt.set_sky(m_sky);
}

void Scene::set_textured_sky(const char* fn_tex, bool srgb, float angle, const glm::vec3& v)
{
	int texId = m_tex_map.findTex(fn_tex, srgb);
	delete m_sky;
	TexturedSkyBox *t_sky = new TexturedSkyBox(texId);
	t_sky->rotate(angle, v);
	m_sky = t_sky;	
	m_pt.set_sky(m_sky);
}

void Scene::add_colored_sphere(const glm::mat4x4& model, const Material& material)
{
	ColoredUnitSphere* sphere = new ColoredUnitSphere(model, material);
	m_geos.push_back(sphere);
	m_pt.add_geometry(sphere);
}

void Scene::add_colored_cube(const glm::mat4x4& model, const Material& material)
{
	static std::vector<ColoredIndexedTriangleList::Vertex> cube_vertices =
	{
		{{-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f } },
		{{-1.0f, -1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f } },
		{{-1.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f } },
		{{-1.0f, 1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f } },

		{{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f }},
		{{1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f } },
		{{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f } },
		{{-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f } },

		{{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f } },
		{{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f } },
		{{1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f } },
		{{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f } },

		{{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f } },
		{{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f } },
		{{-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f } },
		{{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f } },

		{{1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f } },
		{{1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f } },
		{{-1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f } },
		{{-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f } },

		{{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f } },
		{{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f } },
		{{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f } },
		{{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f } },
	};

	static std::vector<unsigned> cube_indices =
	{
		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		16, 17, 18,
		16, 18, 19,

		20, 21, 22,
		20, 22, 23
	};


	ColoredIndexedTriangleList* cube = new ColoredIndexedTriangleList(model, cube_vertices, cube_indices, material);
	m_geos.push_back(cube);
	m_pt.add_geometry(cube);

}

void Scene::add_textured_sphere(const glm::mat4x4& model, const char* fn_tex, bool srgb, const glm::vec3& color)
{
	int texId = m_tex_map.findTex(fn_tex, srgb);
	TexturedUnitSphere* sphere = new TexturedUnitSphere(model, texId, color);
	m_geos.push_back(sphere);
	m_pt.add_geometry(sphere);
}

void Scene::add_wavefront_object(const glm::mat4x4& model, const char* path, const char* fn)
{
	std::string str_path = path;
	str_path += "/";
	std::string fn_obj = str_path + fn;	

	tinyobj::attrib_t                attrib;
	std::vector<tinyobj::shape_t>    shapes;
	std::vector<tinyobj::material_t> materials;
	std::string                      err;

	tinyobj::LoadObj(&attrib, &shapes, &materials, &err, fn_obj.c_str(), path);

	std::vector<WavefrontIndexedTriangleList::Material> materials_in(materials.size());

	for (size_t i = 0; i < materials.size(); i++)
	{
		materials_in[i].diffuse = glm::vec3(materials[i].diffuse[0], materials[i].diffuse[1], materials[i].diffuse[2]);
		materials_in[i].specular = glm::vec3(materials[i].specular[0], materials[i].specular[1], materials[i].specular[2]);
		materials_in[i].emission = glm::vec3(materials[i].emission[0], materials[i].emission[1], materials[i].emission[2]);
		materials_in[i].shininess = materials[i].shininess;

		if (!materials[i].diffuse_texname.empty())
			materials_in[i].texId_diffuse = m_tex_map.findTex((str_path+materials[i].diffuse_texname).c_str());
		else
			materials_in[i].texId_diffuse = -1;

		if (!materials[i].specular_texname.empty())
			materials_in[i].texId_specular = m_tex_map.findTex((str_path+materials[i].specular_texname).c_str());
		else
			materials_in[i].texId_specular = -1;

		if (!materials[i].emissive_texname.empty())
			materials_in[i].texId_emission = m_tex_map.findTex((str_path+materials[i].emissive_texname).c_str());
		else
			materials_in[i].texId_emission = -1;

		if (!materials[i].bump_texname.empty())
			materials_in[i].texId_bumpmap = m_tex_map.findTex((str_path+materials[i].bump_texname).c_str(), false);
		else
			materials_in[i].texId_bumpmap = -1;

		if (!materials[i].alpha_texname.empty())
			materials_in[i].texId_mask = m_tex_map.findTex(materials[i].alpha_texname.c_str(), false);
		else
			materials_in[i].texId_mask = -1;

		int mask = 0;
		if (materials[i].diffuse[0] > 0.0f || materials[i].diffuse[1] > 0.0f || materials[i].diffuse[2] > 0.0f)
		{
			mask |= 1;
		}
		else if (materials_in[i].texId_diffuse >= 0)
		{
			materials_in[i].diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
			mask |= 1;
		}
		if (materials[i].specular[0] > 0.0f || materials[i].specular[1] > 0.0f || materials[i].specular[2] > 0.0f) mask |= 2;
		if (materials[i].emission[0] > 0.0f || materials[i].emission[1] > 0.0f || materials[i].emission[2] > 0.0f) mask |= 4;
		materials_in[i].mask = mask;
	}

	std::vector<glm::vec3> positions(attrib.vertices.size() / 3);
	for (size_t i = 0; i < positions.size(); i++)
	{
		float* vp = &attrib.vertices[3 * i];
		positions[i] = glm::vec3(vp[0], vp[1], vp[2]);
	}

	std::vector<glm::vec3> normals(attrib.normals.size() / 3);
	for (size_t i = 0; i < normals.size(); i++)
	{
		float* np = &attrib.normals[3 * i];
		normals[i] = glm::vec3(np[0], np[1], np[2]);
	}

	std::vector<glm::vec2> tex_coords(attrib.texcoords.size() / 2);
	for (size_t i = 0; i < tex_coords.size(); i++)
	{
		float* tp = &attrib.texcoords[2 * i];
		tex_coords[i] = glm::vec2(tp[0], 1.0f - tp[1]);
	}

	std::vector<WavefrontIndexedTriangleList::Index> indices;
	std::vector<int> materials_ids;

	for (size_t i = 0; i < shapes.size(); i++)
	{
		tinyobj::shape_t& shape = shapes[i];

		for (size_t j = 0; j < shape.mesh.indices.size(); j++)
		{
			tinyobj::index_t& t_index = shape.mesh.indices[j];
			WavefrontIndexedTriangleList::Index index;
			index.vertex_index = t_index.vertex_index;
			index.normal_index = t_index.normal_index;
			index.texcoord_index = t_index.texcoord_index;
			indices.push_back(index);
		}

		for (size_t j = 0; j < shape.mesh.material_ids.size(); j++)
			materials_ids.push_back(shape.mesh.material_ids[j]);
	}

	WavefrontIndexedTriangleList* witl = new WavefrontIndexedTriangleList(model, positions, normals, tex_coords, indices, materials_in, materials_ids.data());
	m_geos.push_back(witl);
	m_pt.add_geometry(witl);
}

void Scene::add_sphere_light(const glm::vec3& center, float r, const glm::vec3& color)
{
	SphereLight* light = new SphereLight(center, r, color);
	m_geos.push_back(light);
	m_pt.add_geometry(light);
}



