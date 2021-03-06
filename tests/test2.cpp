#include "PathTracer.h"
#include "ColoredIndexedTriangleList.h"
#include "ColoredUnitSphere.h"
#include "UnitSphereCheckerTex.h"
#include "Timing.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

typedef ColoredIndexedTriangleList TriangleMesh;
typedef ColoredUnitSphere SphereA;
typedef UnitSphereCheckerTex SphereB;


double rand01()
{
	return (double)rand() / (double)RAND_MAX;
}

int main()
{
	srand(time(0));

	const int view_width = 900;
	const int view_height = 600;

	printf("Generating scene..\n");
	double time0 = GetTime();

	glm::mat4x4 identity = glm::identity<glm::mat4x4>();

	std::vector<Geometry*> geometries;


	{
		glm::mat4x4 model = glm::translate(identity, glm::vec3(0.0f, -1000.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1000.0f, 1000.0f, 1000.0f));
		SphereB* sphere = new SphereB(model, 0.3f, {0.2f, 0.3f, 0.1f}, { 0.9f, 0.9f, 0.9f });
		geometries.push_back(sphere);
	}

	{
		glm::mat4x4 model = glm::translate(identity, glm::vec3(0.0f, 1.0f, 0.0f));
		SphereA* sphere = new SphereA(model, { dielectric, { 1.0f, 1.0f, 1.0f }, 0.0f, 1.5f });
		geometries.push_back(sphere);
	}

	{
		glm::mat4x4 model = glm::translate(identity, glm::vec3(-4.0f, 1.0f, 0.0f));
		SphereA* sphere = new SphereA(model, { lambertian, { 0.4f, 0.2f, 0.1f } });
		geometries.push_back(sphere);
	}

	{
		glm::mat4x4 model = glm::translate(identity, glm::vec3(4.0f, 1.0f, 0.0f));
		SphereA* sphere = new SphereA(model, { metal, { 0.7f, 0.6f, 0.5f } });
		geometries.push_back(sphere);
	}

	for (int a = -11; a < 11; a++)
		for (int b = -11; b < 11; b++)
		{
			double choose_mat = rand01();
			glm::vec3 center = { (float)a + 0.9f*rand01(), 0.2f, (float)b + 0.9f*rand01() };
			float dis = (center.x - 4.0f)*(center.x - 4.0f) + center.z*center.z;
			if (dis < 1.0f)	continue;
			dis = center.x*center.x + center.z*center.z;
			if (dis < 1.0f) continue;

			glm::mat4x4 model = glm::translate(identity, center);
			model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

			if (choose_mat < 0.75f)
			{
				SphereA* sphere = new SphereA(model, { lambertian, { rand01()*rand01(), rand01()*rand01(), rand01()*rand01() } });
				geometries.push_back(sphere);
			}
			else if (choose_mat < 0.90f)
			{
				SphereA* sphere = new SphereA(model, { metal, { 0.5f*(1.0f + rand01()), 0.5f*(1.0f + rand01()),0.5f*(1.0f + rand01()) } });
				geometries.push_back(sphere);
			}
			else
			{
				SphereA* sphere = new SphereA(model, { dielectric, { 1.0f, 1.0f, 1.0f },  0.0f, 1.5f });
				geometries.push_back(sphere);
			}

		}
	double time1 = GetTime();
	printf("Done generating scene.. %f secs\n", time1 - time0);


	Image target(view_width, view_height, nullptr);

	PathTracer pt;
	pt.set_target(&target);

	for (size_t i = 0; i < geometries.size(); i++)
		pt.add_geometry(geometries[i]);

	pt.set_camera({ 15.0f, 3.0f, 3.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 20.0f, 0.2f, 12.0f);
	pt.trace(100);

	unsigned char* hbuffer = (unsigned char*)malloc(view_width * view_height * 3);
	target.to_host_srgb(hbuffer);
	stbi_write_png("test2.png", view_width, view_height, 3, hbuffer, view_width * 3);
	free(hbuffer);

	for (size_t i = 0; i < geometries.size(); i++)
		delete geometries[i];

	system("pause");

	return 0;
}

