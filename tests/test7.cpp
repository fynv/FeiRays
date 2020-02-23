#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "PathTracer.h"
#include "wavefront_obj.h"
#include "TexturedUnitSphere.h"
#include "WavefrontIndexedTriangleList.h"

#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "dds_reader.hpp"

#ifndef PI
#define PI 3.1415926f
#endif

int main()
{
	const int view_width = 900;
	const int view_height = 600;

	PathTracer pt;

	Image target(view_width, view_height);
	pt.set_target(&target);

	int cubeWidth, cubeHeight, cubeChannels, isCube;
	void *cube_pixels = dds_load("../data/sky_cube.dds", &cubeWidth, &cubeHeight, &cubeChannels, &isCube);
	RGBACubemap cubemap(cubeWidth, cubeHeight, cube_pixels, false);
	dds_free(cube_pixels);

	int id_skybox = pt.add_cubemap(&cubemap);
	TexturedSkyBox skybox(id_skybox);
	pt.set_sky(&skybox);
	pt.add_sunlight({ 1.0f, 1.0f, 1.0f }, 0.05, { 5000.0f, 5000.0f, 5000.0f });

	glm::mat4x4 identity = glm::identity<glm::mat4x4>();

	//SphereLight light1({ 0.0f, 200.0f, 0.0f }, 10.0f, { 10.0f, 10.0f, 10.0f });
	//pt.add_geometry(&light1);

	WavefrontObject obj(pt, "../data/sponza", "SponzaNoFlag.obj", identity);

	pt.add_geometry(obj.get_geo());
	pt.set_camera({ -1000.0f, 1000.0f, 0.0f }, { 0.0f, 350.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f, 10.0f, 1400.0f);

	pt.trace(1000, 50);

	unsigned char* hbuffer = (unsigned char*)malloc(view_width * view_height * 3);
	target.to_host_srgb(hbuffer, 5.0f);
	stbi_write_png("test7.png", view_width, view_height, 3, hbuffer, view_width * 3);
	free(hbuffer);

	system("pause");


	return 0;
}