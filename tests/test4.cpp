#include "PathTracer.h"
#include "ColoredIndexedTriangleList.h"
#include "ColoredUnitSphere.h"
#include "TexturedUnitSphere.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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

	const int view_width = 900;
	const int view_height = 600;

	Image target(view_width, view_height);

	PathTracer pt;
	pt.set_target(&target);

	GradientSky sky({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f });
	pt.set_sky(&sky);

	glm::mat4x4 identity = glm::identity<glm::mat4x4>();

	glm::mat4x4 model0 = glm::translate(identity, glm::vec3(0.0f, -1000.0f, 0.0f));
	model0 = glm::scale(model0, glm::vec3(1000.0f, 1000.0f, 1000.0f));
	ColoredUnitSphere sphere0(model0, { lambertian, { 0.4f, 0.4f, 0.6f } });
	pt.add_geometry(&sphere0);

	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load("../data/moon_map.jpg", &texWidth, &texHeight, &texChannels, 4);
	RGBATexture tex(texWidth, texHeight, pixels);
	stbi_image_free(pixels);
	int tex_id = pt.add_texture(&tex);

	glm::mat4x4 model1 = glm::translate(identity, glm::vec3(0.0f, 2.0f, 0.0f));
	model1 = glm::scale(model1, glm::vec3(2.0f, 2.0f, 2.0f));
	model1 = glm::rotate(model1, PI / 3.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	TexturedUnitSphere sphere1(model1, tex_id, { 0.8f, 0.8f, 0.6f });
	pt.add_geometry(&sphere1);

	glm::mat4x4 model2 = glm::translate(identity, glm::vec3(4.0f, 2.0f, -2.01f));
	model2 = glm::scale(model2, glm::vec3(1.0f, 1.0f, 0.01f));
	ColoredIndexedTriangleList cube2(model2, cube_vertices, cube_indices, { emissive, { 4.0f, 4.0f, 4.0f } });
	pt.add_geometry(&cube2);


	pt.set_camera({ 15.0f, 3.0f, 3.0f }, { 0.0f, 2.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 30.0f, 0.2f, 12.0f);

	pt.trace(100);

	unsigned char* hbuffer = (unsigned char*)malloc(view_width * view_height * 3);
	target.to_host_srgb(hbuffer);
	stbi_write_png("test4.png", view_width, view_height, 3, hbuffer, view_width * 3);
	free(hbuffer);

	system("pause");


	return 0;
}
