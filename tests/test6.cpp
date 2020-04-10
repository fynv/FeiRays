#include "PathTracer.h"
#include "ColoredIndexedTriangleList.h"
#include "ColoredUnitSphere.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#ifndef PI
#define PI 3.1415926f
#endif

typedef ColoredIndexedTriangleList TriangleMesh;
typedef ColoredUnitSphere Sphere;


int main()
{
	std::vector<TriangleMesh::Vertex> cube_vertices =
	{
		{{-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f } },
		{{-1.0f, -1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f } },
		{{-1.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f } },
		{{-1.0f, 1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f } },

		{{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f }},
		{{1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f } },
		{{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f } },
		{{-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f } },

		{{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f } },
		{{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f } },
		{{1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f } },
		{{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f } },

		{{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f } },
		{{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f } },
		{{-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f } },
		{{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f } },

		{{1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f } },
		{{1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f } },
		{{-1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f } },
		{{-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f } },

		{{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f } },
		{{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f } },
		{{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f } },
		{{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f } },
	};

	std::vector<unsigned> cube_indices =
	{
		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		16, 17, 18,
		16, 18, 19,

		20, 21, 22,
		20, 22, 23
	};

	const int view_width = 800;
	const int view_height = 400;

	glm::mat4x4 identity = glm::identity<glm::mat4x4>();

	glm::mat4x4 model0 = glm::translate(identity, glm::vec3(0.0f, 1.0f, -2.0f));
	model0 = glm::rotate(model0, 45.0f / 180.0f*PI, glm::vec3(0.0f, 1.0f, 0.0f));
	TriangleMesh cube0(model0, cube_vertices, cube_indices, { lambertian, { 0.1f, 0.5f, 0.2f } });
	
	glm::mat4x4 model1 = glm::translate(identity, glm::vec3(4.0f, 1.0f, -2.0f));
	model1 = glm::rotate(model1, -45.0f / 180.0f*PI, glm::vec3(0.0f, 1.0f, 0.0f));
	TriangleMesh cube1(model1, cube_vertices, cube_indices, { lambertian, { 0.1f, 0.2f, 0.5f } });

	glm::mat4x4 model2 = glm::translate(identity, glm::vec3(-4.0, 1.0, -2.0));
	model2 = glm::rotate(model2, -45.0f / 180.0f*PI, glm::vec3(0.0, 1.0, 0.0f));
	TriangleMesh cube2(model2, cube_vertices, cube_indices, { lambertian, { 0.8f, 0.3f, 0.3f }});

	glm::mat4x4 model3 = glm::translate(identity, glm::vec3(0.0, -0.2, 0.0));
	model3 = glm::scale(model3, glm::vec3(6.0f, 0.2f, 6.0f));
	TriangleMesh cube3(model3, cube_vertices, cube_indices, { lambertian, { 0.7, 0.7, 0.7 } });

	glm::mat4x4 model4 = glm::translate(identity, glm::vec3(0.0, 1.0, 2.0));
	Sphere sphere4(model4, { lambertian, { 0.1f, 0.2f, 0.5f } });

	glm::mat4x4 model5 = glm::translate(identity, glm::vec3(4.0, 1.0, 2.0));
	Sphere sphere5(model5, { lambertian, { 0.8f, 0.3f, 0.3f } });

	glm::mat4x4 model6 = glm::translate(identity, glm::vec3(-4.0, 1.0, 2.0));
	Sphere sphere6(model6, { lambertian, { 0.1f, 0.5f, 0.2f } });

	Image target(view_width, view_height, nullptr, 1 << 17);

	PathTracer pt;
	
	GradientSky sky({ 0.5f, 0.5f, 0.5f }, { 0.2f, 0.3f, 0.5f });
	pt.set_sky(&sky);
	pt.add_sunlight({ 1.0f, 1.0f, 1.0f }, 0.05, { 100.0f, 100.0f, 100.0f });

	pt.set_target(&target);
	pt.add_geometry(&cube0);
	pt.add_geometry(&cube1);
	pt.add_geometry(&cube2);
	pt.add_geometry(&cube3);
	pt.add_geometry(&sphere4);
	pt.add_geometry(&sphere5);
	pt.add_geometry(&sphere6);
	pt.set_camera({ 0.0f, 8.0f, 8.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f);

	pt.trace(100);

	unsigned char* hbuffer = (unsigned char*)malloc(view_width * view_height * 3);
	target.to_host_srgb(hbuffer);
	stbi_write_png("test6.png", view_width, view_height, 3, hbuffer, view_width * 3);
	free(hbuffer);

	system("pause");

	return 0;
}
