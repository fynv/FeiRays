#include "PathTracer.h"
#include "ColoredIndexedTriangleList.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifndef PI
#define PI 3.1415926f
#endif


int main()
{
	std::vector<ColoredIndexedTriangleList::Vertex> cube_vertices =
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
	const int view_height = 800;

	Image target(view_width, view_height);

	PathTracer pt;
	pt.set_target(&target);

	GradientSky sky({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f });
	pt.set_sky(&sky);

	glm::mat4x4 identity = glm::identity<glm::mat4x4>();

	glm::mat4x4 model0 = glm::translate(identity, glm::vec3(1110.01f, 555.0f, 555.0f));
	model0 = glm::scale(model0, glm::vec3(0.01f, 555.0f, 555.0f));
	ColoredIndexedTriangleList cube0(model0, cube_vertices, cube_indices, { 0.12, 0.45f, 0.15f });
	pt.add_geometry(&cube0);

	glm::mat4x4 model1 = glm::translate(identity, glm::vec3(-0.01f, 555.0f, 555.0f));
	model1 = glm::scale(model1, glm::vec3(0.01f, 555.0f, 555.0f));
	ColoredIndexedTriangleList cube1(model1, cube_vertices, cube_indices, { 0.65, 0.05f, 0.05f });
	pt.add_geometry(&cube1);

	glm::mat4x4 model2 = glm::translate(identity, glm::vec3(555.0f, 1108.0f, 555.0f));
	model2 = glm::scale(model2, glm::vec3(130.0f, 0.01f, 105.0f));
	ColoredIndexedTriangleList cube2(model2, cube_vertices, cube_indices, { 15.0f, 15.0f, 15.0f }, emissive);
	pt.add_geometry(&cube2);

	glm::mat4x4 model3 = glm::translate(identity, glm::vec3(555.0f, 1110.01f, 555.0f));
	model3 = glm::scale(model3, glm::vec3(550.0f, 0.01f, 555.0f));
	ColoredIndexedTriangleList cube3(model3, cube_vertices, cube_indices, { 0.73f, 0.73f, 0.73f });
	pt.add_geometry(&cube3);

	glm::mat4x4 model4 = glm::translate(identity, glm::vec3(555.0f, -0.01f, 555.0f));
	model4 = glm::scale(model4, glm::vec3(550.0f, 0.01f, 555.0f));
	ColoredIndexedTriangleList cube4(model4, cube_vertices, cube_indices, { 0.73f, 0.73f, 0.73f });
	pt.add_geometry(&cube4);

	glm::mat4x4 model5 = glm::translate(identity, glm::vec3(555.0f, 555.0f, 1110.01f));
	model5 = glm::scale(model5, glm::vec3(550.0f, 555.0f, 0.01f));
	ColoredIndexedTriangleList cube5(model5, cube_vertices, cube_indices, { 0.73f, 0.73f, 0.73f });
	pt.add_geometry(&cube5);

	glm::mat4x4 model6 = glm::translate(identity, glm::vec3(425.0f, 165.0f, 295.0f));
	model6 = glm::scale(model6, glm::vec3(165.0f, 165.0f, 165.0f));
	model6 = glm::rotate(model6, -18.0f/180.0f*PI, { 0.0f, 1.0f, 0.0f });
	ColoredIndexedTriangleList cube6(model6, cube_vertices, cube_indices, { 0.73f, 0.73f, 0.73f });
	pt.add_geometry(&cube6);

	glm::mat4x4 model7 = glm::translate(identity, glm::vec3(695.0f, 330.0f, 755.0f));
	model7 = glm::scale(model7, glm::vec3(165.0f, 330.0f, 165.0f));
	model7 = glm::rotate(model7, 15.0f / 180.0f*PI, { 0.0f, 1.0f, 0.0f });
	ColoredIndexedTriangleList cube7(model7, cube_vertices, cube_indices, { 0.73f, 0.73f, 0.73f });
	pt.add_geometry(&cube7);

	pt.set_camera({ 556.0f, 556.0f, -1600.0f }, { 556.0f, 556.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 40.0f);

	pt.trace(100);

	unsigned char* hbuffer = (unsigned char*)malloc(view_width * view_height * 3);
	target.to_host_srgb(hbuffer);
	FILE* fp = fopen("test5.raw", "wb");
	fwrite(hbuffer, 1, view_width * view_height * 3, fp);
	fclose(fp);
	free(hbuffer);


	system("pause");



	return 0;

}
