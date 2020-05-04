#include "api.h"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#define _USE_MATH_DEFINES
#include <math.h>

void* n_vec3_create(float x, float y, float z)
{
	glm::vec3* v = new glm::vec3(x,y,z);
	return v;
}

void n_vec3_destroy(void* cptr)
{
	glm::vec3* v = (glm::vec3*)cptr;
	delete v;
}

void* n_transform_create()
{
	glm::mat4x4* trans = new glm::mat4x4;
	*trans = glm::identity<glm::mat4x4>();
	return trans;
}

void n_transform_destroy(void* cptr)
{
	glm::mat4x4* trans = (glm::mat4x4*)cptr;
	delete trans;
}

void n_transform_translate(void* ptr_trans, void* ptr_vec)
{
	glm::mat4x4* trans = (glm::mat4x4*)ptr_trans;
	glm::vec3* vec = (glm::vec3*)ptr_vec;
	*trans = glm::translate(*trans, *vec);
}

void n_transform_rotate(void* ptr_trans, float angle, void* ptr_vec)
{
	glm::mat4x4* trans = (glm::mat4x4*)ptr_trans;
	float rad = angle / 180.0f * (float)M_PI;
	glm::vec3* vec = (glm::vec3*)ptr_vec;
	*trans = glm::rotate(*trans, rad, *vec);
}

void n_transform_scale(void* ptr_trans, void* ptr_vec)
{
	glm::mat4x4* trans = (glm::mat4x4*)ptr_trans;
	glm::vec3* vec = (glm::vec3*)ptr_vec;
	*trans = glm::scale(*trans, *vec);
}


