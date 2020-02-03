#include "volk.h"

#include "PathTracer.h"
#include "ColoredIndexedTriangleList.h"
#include "ColoredUnitSphere.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#ifndef PI
#define PI 3.1415926f
#endif

typedef ColoredIndexedTriangleList TriangleMesh;
typedef ColoredUnitSphere Sphere;


int main()
{
	std::vector<Vertex> cube_vertices =
	{
		{{-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f }, {0.0f, 0.0f} },
		{{-1.0f, -1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f }, {1.0f, 0.0f} },
		{{-1.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f }, {1.0f, 1.0f} },
		{{-1.0f, 1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f }, {0.0f, 1.0f} },

		{{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f }, {0.0f, 0.0f} },
		{{1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f }, {1.0f, 0.0f} },
		{{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f }, {1.0f, 1.0f} },
		{{-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f }, {0.0f, 1.0f} },

		{{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f }, {0.0f, 0.0f} },
		{{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f }, {1.0f, 0.0f} },
		{{1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f }, {1.0f, 1.0f} },
		{{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f }, {0.0f, 1.0f} },

		{{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f }, {0.0f, 0.0f} },
		{{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f }, {1.0f, 0.0f} },
		{{-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f }, {1.0f, 1.0f} },
		{{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f }, {0.0f, 1.0f} },

		{{1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f }, {0.0f, 0.0f} },
		{{1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f }, {1.0f, 0.0f} },
		{{-1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f }, {1.0f, 1.0f} },
		{{-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f }, {0.0f, 1.0f} },

		{{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f }, {0.0f, 0.0f} },
		{{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f }, {1.0f, 0.0f} },
		{{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f }, {1.0f, 1.0f} },
		{{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f }, {0.0f, 1.0f} },
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

	glm::mat4x4 model0 = glm::translate(identity, glm::vec3(0.0, 1.0, -2.0));
	model0 = glm::rotate(model0, 45.0f / 180.0f*PI, glm::vec3(0.0, 1.0, 0.0f));

	TriangleMesh cube0(model0, cube_vertices, cube_indices, { 0.8, 0.6, 0.8 });

	glm::mat4x4 model1 = glm::translate(identity, glm::vec3(4.0, 1.0, -2.0));
	model1 = glm::rotate(model1, -45.0f / 180.0f*PI, glm::vec3(0.0, 1.0, 0.0f));
	TriangleMesh cube1(model1, cube_vertices, cube_indices, { 0.8, 0.8, 0.6 });

	glm::mat4x4 model2 = glm::translate(identity, glm::vec3(-4.0, 1.0, -2.0));
	model2 = glm::rotate(model2, -45.0f / 180.0f*PI, glm::vec3(0.0, 1.0, 0.0f));
	TriangleMesh cube2(model2, cube_vertices, cube_indices, { 0.6, 0.8, 0.8 });

	glm::mat4x4 model3 = glm::translate(identity, glm::vec3(0.0, -0.2, 0.0));
	model3 = glm::scale(model3, glm::vec3(6.0f, 0.2f, 6.0f));
	TriangleMesh cube3(model3, cube_vertices, cube_indices, { 1.0, 1.0, 1.0 });

	glm::mat4x4 model4 = glm::translate(identity, glm::vec3(0.0, 1.0, 2.0));
	Sphere sphere4(model4, { 0.8, 0.6, 0.8 });

	glm::mat4x4 model5 = glm::translate(identity, glm::vec3(4.0, 1.0, 2.0));
	Sphere sphere5(model5, { 0.6, 0.8, 0.8 });

	glm::mat4x4 model6 = glm::translate(identity, glm::vec3(-4.0, 1.0, 2.0));
	Sphere sphere6(model6, { 0.8, 0.8, 0.6 });

	Image target(view_width, view_height);

	PathTracer pt;
	pt.set_target(&target);
	pt.add_geometry(&cube0);
	pt.add_geometry(&cube1);
	pt.add_geometry(&cube2);
	pt.add_geometry(&cube3);
	pt.add_geometry(&sphere4);
	pt.add_geometry(&sphere5);
	pt.add_geometry(&sphere6);
	pt.set_camera({ 0.0f, 8.0f, 8.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f);

	pt.trace();

	float* hbuffer = (float*)malloc(view_width * view_height * sizeof(float) * 4);
	target.to_host(hbuffer);
	FILE* fp = fopen("dump.raw", "wb");
	for (unsigned i = 0; i < view_width * view_height; i++)
	{
		unsigned char pix[3];
		pix[0] = (unsigned char)(hbuffer[i * 4] * 255.0f);
		pix[1] = (unsigned char)(hbuffer[i * 4 + 1] * 255.0f);
		pix[2] = (unsigned char)(hbuffer[i * 4 + 2] * 255.0f);
		fwrite(pix, 1, 3, fp);
	}
	fclose(fp);
	free(hbuffer);

	system("pause");

	return 0;
}
