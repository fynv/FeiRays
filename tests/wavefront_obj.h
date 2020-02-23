#pragma once

#include "PathTracer.h"
#include "texture_map.h"


class WavefrontIndexedTriangleList;

class WavefrontObject
{
public:
	WavefrontObject(PathTracer& pt, const char* path, const char* fn, const glm::mat4x4& model);
	~WavefrontObject();

	WavefrontIndexedTriangleList* get_geo() const { return m_witl; }


private:
	TextureMap m_tex_map;
	WavefrontIndexedTriangleList* m_witl;

};
