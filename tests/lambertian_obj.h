#pragma once

#include "PathTracer.h"

class TexturedTriangleList;

class LambertianObject
{
public:
	LambertianObject(PathTracer& pt, const char* path, const char* fn, const glm::mat4x4& model);
	~LambertianObject();

	TexturedTriangleList* get_geo() const { return m_ttl; }

private:
	std::vector<RGBATexture*> m_textures;
	TexturedTriangleList* m_ttl;

};
