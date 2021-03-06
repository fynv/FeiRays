#include "texture_map.h"
#include "PathTracer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "dds_reader.hpp"

#include "string.h"

TextureMap::TextureMap(PathTracer* pt)
{
	m_pt = pt;
}

TextureMap::~TextureMap()
{
	for (size_t i = 0; i < m_textures.size(); i++)
		delete m_textures[i];
	for (size_t i = 0; i < m_cubemaps.size(); i++)
		delete m_cubemaps[i];
}

inline const char* extension(const char* fn)
{
	return fn + strlen(fn) - 3;
}


inline std::string fix_path(const std::string& in)
{
	std::string ret = "";
	for (size_t i=0; i<in.size(); i++)
	{
		if (in[i]=='\\') ret+="/";
		else ret += in[i];
	}
	return ret;
}

int TextureMap::findTex(const char* texname, bool srgb)
{
	std::string fn_tex = fix_path(texname);

	int ret;
	auto iter = find(fn_tex);
	if (iter == end())
	{
		int texWidth, texHeight, texChannels, isCube;

		if (strcmp(extension(fn_tex.c_str()), "dds") == 0)
		{
			void *pixels = dds_load(fn_tex.c_str(), &texWidth, &texHeight, &texChannels, &isCube);
			if (isCube)
			{
				RGBACubemap* cube = new RGBACubemap(texWidth, texHeight, pixels, srgb);
				ret = m_pt->add_cubemap(cube);
				m_cubemaps.push_back(cube);
			}
			else
			{
				RGBATexture* tex = new RGBATexture(texWidth, texHeight, pixels, srgb);
				ret = m_pt->add_texture(tex);
				m_textures.push_back(tex);
			}
			dds_free(pixels);
		}
		else
		{
			stbi_uc* pixels = stbi_load(fn_tex.c_str(), &texWidth, &texHeight, &texChannels, 4);
			RGBATexture* tex = new RGBATexture(texWidth, texHeight, pixels, srgb);
			ret = m_pt->add_texture(tex);
			m_textures.push_back(tex);
			stbi_image_free(pixels);
		}

		(*this)[fn_tex] = ret;
	}
	else
	{
		ret = iter->second;
	}
	return ret;
}
