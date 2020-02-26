#pragma once

#include <vector>
#include <unordered_map>

class PathTracer;
class RGBATexture;
class RGBACubemap;

class TextureMap : private std::unordered_map<std::string, int>
{
public:
	TextureMap(PathTracer* pt);
	virtual ~TextureMap();

	int findTex(const char* texname, bool srgb = true);
	

private:
	PathTracer* m_pt;
	std::vector<RGBATexture*> m_textures;
	std::vector<RGBACubemap*> m_cubemaps;
};

