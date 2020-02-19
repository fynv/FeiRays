#pragma once

#include <glm.hpp>

enum MaterialType
{
	lambertian,
	metal,
	dielectric,
	emissive,
	foggy
};

struct Material
{
	MaterialType type = lambertian;
	glm::vec3 color = { 1.0f, 1.0f, 1.0f };
	float fuzz = 0.0f; 
	float ref_idx = 0.0f;
	float density = 0.0f;
};

