#include "texture_map.h"
#include "PathTracer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

TextureMap::TextureMap(PathTracer* pt, const char* path)
{
	m_pt = pt;
	m_path = path;
}

TextureMap::~TextureMap()
{
	for (size_t i = 0; i < m_textures.size(); i++)
		delete m_textures[i];
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
	int ret;
	auto iter = find(texname);
	if (iter == end())
	{
		std::string fn_tex = m_path + "/" + texname;
		fn_tex = fix_path(fn_tex);
		int texWidth, texHeight, texChannels;

		stbi_uc* pixels = stbi_load(fn_tex.c_str(), &texWidth, &texHeight, &texChannels, 4);

		RGBATexture* tex = new RGBATexture(texWidth, texHeight, pixels, srgb);
		ret = m_pt->add_texture(tex);
		m_textures.push_back(tex);

		(*this)[texname] = ret;

		stbi_image_free(pixels);
	}
	else
	{
		ret = iter->second;
	}
	return ret;
}
