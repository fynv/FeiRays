#pragma once

#include "PathTracer.h"
#include "texture_map.h"

class TexturedTriangleList;

class LambertianObject
{
public:
	LambertianObject(PathTracer& pt, const char* path, const char* fn, const glm::mat4x4& model);
	~LambertianObject();

	TexturedTriangleList* get_geo() const { return m_ttl; }

private:
	TextureMap m_tex_map;
	TexturedTriangleList* m_ttl;

};
