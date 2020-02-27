#include <string>
#include <unordered_map>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "PathTracer.h"
#include "WavefrontIndexedTriangleList.h"

#include "wavefront_obj.h"

WavefrontObject::WavefrontObject(PathTracer& pt, const char* path, const char* fn, const glm::mat4x4& model)
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
	
	std::vector<WavefrontIndexedTriangleList::Material> materials_in(materials.size());

	for (size_t i = 0; i < materials.size(); i++)
	{
		materials_in[i].diffuse = glm::vec3(materials[i].diffuse[0], materials[i].diffuse[1], materials[i].diffuse[2]);
		materials_in[i].specular = glm::vec3(materials[i].specular[0], materials[i].specular[1], materials[i].specular[2]);
		materials_in[i].emission = glm::vec3(materials[i].emission[0], materials[i].emission[1], materials[i].emission[2]);
		materials_in[i].shininess = materials[i].shininess;

		if (!materials[i].diffuse_texname.empty())
			materials_in[i].texId_diffuse = m_tex_map.findTex(materials[i].diffuse_texname.c_str());
		else
			materials_in[i].texId_diffuse = -1;

		if (!materials[i].specular_texname.empty())
			materials_in[i].texId_specular = m_tex_map.findTex(materials[i].specular_texname.c_str());
		else
			materials_in[i].texId_specular = -1;

		if (!materials[i].emissive_texname.empty())
			materials_in[i].texId_emission = m_tex_map.findTex(materials[i].emissive_texname.c_str());
		else
			materials_in[i].texId_emission = -1;

		if (!materials[i].bump_texname.empty())
			materials_in[i].texId_bumpmap = m_tex_map.findTex(materials[i].bump_texname.c_str(), false);
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

	std::vector<glm::vec3> positions(attrib.vertices.size()/3);
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

	m_witl = new WavefrontIndexedTriangleList(model, positions, normals, tex_coords, indices, materials_in, materials_ids.data());
}

WavefrontObject::~WavefrontObject()
{
	delete m_witl;
}