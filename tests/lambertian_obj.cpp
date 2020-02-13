#include <string>
#include <unordered_map>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "PathTracer.h"
#include "TexturedTriangleList.h"

#include "lambertian_obj.h"

LambertianObject::LambertianObject(PathTracer& pt, const char* path, const char* fn, const glm::mat4x4& model)
{
	std::string fn_obj = path;
	fn_obj += "/";
	fn_obj += fn;

	tinyobj::attrib_t                attrib;
	std::vector<tinyobj::shape_t>    shapes;
	std::vector<tinyobj::material_t> materials;
	std::string                      err;

	tinyobj::LoadObj(&attrib, &shapes, &materials, &err, fn_obj.c_str(), path);

	std::unordered_map<std::string, int> tex_map;

	std::vector<TexturedTriangleList::Material> materials_in(materials.size());

	for (size_t i = 0; i < materials.size(); i++)
	{
		materials_in[i].diffuse = glm::vec3(materials[i].diffuse[0], materials[i].diffuse[1], materials[i].diffuse[2]);
		if (!materials[i].diffuse_texname.empty())
		{
			auto iter = tex_map.find(materials[i].diffuse_texname);
			if (iter == tex_map.end())
			{
				std::string fn_tex = path;
				fn_tex += "/";
				fn_tex += materials[i].diffuse_texname;

				int texWidth, texHeight, texChannels;

				stbi_uc* pixels = stbi_load(fn_tex.c_str(), &texWidth, &texHeight, &texChannels, 4);

				RGBATexture* tex = new RGBATexture(texWidth, texHeight, pixels);
				materials_in[i].textureId = pt.add_texture(tex);
				m_textures.push_back(tex);

				tex_map[materials[i].diffuse_texname] = materials_in[i].textureId;

				stbi_image_free(pixels);
			}
			else
			{
				materials_in[i].textureId = iter->second;
			}
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
			float* tp = &attrib.texcoords[2 * index.texcoord_index + 0];
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
	for (size_t i = 0; i < m_textures.size(); i++)
		delete m_textures[i];
}
