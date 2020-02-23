#pragma once

#include <unordered_map>

class PathTracer;
class RGBATexture;

class TextureMap : private std::unordered_map<std::string, int>
{
public:
	TextureMap(PathTracer* pt, const char* path);
	virtual ~TextureMap();

	int findTex(const char* texname);
	

private:
	PathTracer* m_pt;
	std::string m_path;
	std::vector<RGBATexture*> m_textures;	
};

