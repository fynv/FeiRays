#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "PathTracer.h"
#include "lambertian_obj.h"
#include "TexturedTriangleList.h"
#include "ColoredUnitSphere.h"
#include "TexturedUnitSphere.h"

#include "stb_image.h"

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

	int id_skybox= pt.add_cubemap(&cubemap);
	TexturedSkyBox skybox(id_skybox);
	pt.set_sky(&skybox);

	glm::mat4x4 identity = glm::identity<glm::mat4x4>();

	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load("../data/moon_map.jpg", &texWidth, &texHeight, &texChannels, 4);
	RGBATexture tex(texWidth, texHeight, pixels);
	stbi_image_free(pixels);
	int tex_id = pt.add_texture(&tex);

	glm::mat4x4 model0 = glm::translate(identity, glm::vec3(0.0f, -100.0f, 0.0f));
	model0 = glm::scale(model0, glm::vec3(100.0f, 100.0f, 100.0f));
	model0 = glm::rotate(model0, PI / 6.0f, glm::vec3(1.0f, 0.0f, 1.0f));
	TexturedUnitSphere sphere0(model0, tex_id);
	pt.add_geometry(&sphere0);

	glm::mat4x4 model1 = glm::translate(identity, glm::vec3(0.0f, 1.0f, 3.0f));
	ColoredUnitSphere sphere1(model1, { 1.0f, 1.0f, 1.0f }, dielectric, 0.0f, 1.5f);
	pt.add_geometry(&sphere1);

	glm::mat4x4 model2 = glm::translate(identity, glm::vec3(-3.0f, 1.0f, -1.5f));
	ColoredUnitSphere sphere2(model2, { 0.7f, 0.6f, 0.5f }, metal);
	pt.add_geometry(&sphere2);

	LambertianObject obj(pt, "../data/Medieval_building", "Medieval_building.obj", identity);
	pt.add_geometry(obj.get_geo());
	pt.set_camera({ -12.0f, 6.0f, 12.0f }, { 0.0f, 1.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 30.0f, 0.2f, 16.0f);

	pt.trace(100);

	float* hbuffer = (float*)malloc(view_width * view_height * sizeof(float) * 4);
	target.to_host(hbuffer);
	FILE* fp = fopen("test3.raw", "wb");
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


