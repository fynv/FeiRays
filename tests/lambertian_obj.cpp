#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "PathTracer.h"
#include "TexturedTriangleList.h"

#include "lambertian_obj.h"

LambertianObject::LambertianObject(PathTracer& pt, const char* path, const char* fn, const glm::mat4x4& model)
	: m_tex_map(&pt, path)
{
	std::string fn_obj = path;
	fn_obj += "/";
	fn_obj += fn;

	tinyobj::attrib_t                attrib;
	std::vector<tinyobj::shape_t>    shapes;
	std::vector<tinyobj::material_t> materials;
	std::string                      err;

	tinyobj::LoadObj(&attrib, &shapes, &materials, &err, fn_obj.c_str(), path);

	std::vector<TexturedTriangleList::Material> materials_in(materials.size());

	for (size_t i = 0; i < materials.size(); i++)
	{
		materials_in[i].diffuse = glm::vec3(materials[i].diffuse[0], materials[i].diffuse[1], materials[i].diffuse[2]);
		if (!materials[i].diffuse_texname.empty())
		{
			materials_in[i].textureId = m_tex_map.findTex(materials[i].diffuse_texname.c_str());
		}
		else
		{
			materials_in[i].textureId = -1;
		}
	}

	std::vector<TexturedTriangleList::Vertex> vertices;
	std::vector<int> materials_ids;

	for (size_t i = 0; i < shapes.size(); i++)
	{
		tinyobj::shape_t& shape = shapes[i];

		for (size_t j = 0; j < shape.mesh.indices.size(); j++)
		{
			TexturedTriangleList::Vertex vertex;

			tinyobj::index_t& index = shape.mesh.indices[j];
			float* vp = &attrib.vertices[3 * index.vertex_index];
			vertex.Position = glm::vec3(vp[0], vp[1], vp[2]);
			float* np = &attrib.normals[3 * index.normal_index];
			vertex.Normal = glm::vec3(np[0], np[1], np[2]);
			float* tp = &attrib.texcoords[2 * index.texcoord_index];
			vertex.TexCoord = glm::vec2(tp[0], tp[1]);
			vertices.push_back(vertex);
		}

		for (size_t j = 0; j < shape.mesh.material_ids.size(); j++)
			materials_ids.push_back(shape.mesh.material_ids[j]);
	}

	m_ttl = new TexturedTriangleList(model, vertices, materials_in, materials_ids.data());
}

LambertianObject::~LambertianObject()
{
	delete m_ttl;
}
