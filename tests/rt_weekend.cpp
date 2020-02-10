#include "volk.h"

#include "PathTracer.h"
#include "ColoredIndexedTriangleList.h"
#include "ColoredUnitSphere.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include <vector>

typedef ColoredIndexedTriangleList TriangleMesh;
typedef ColoredUnitSphere Sphere;

double rand01()
{
	return (double)rand() / (double)RAND_MAX;
}

int main()
{
	srand(time(0));

	const int view_width = 900;
	const int view_height = 600;


	glm::mat4x4 identity = glm::identity<glm::mat4x4>();

	std::vector<Sphere*> spheres;


	{
		glm::mat4x4 model = glm::translate(identity, glm::vec3(0.0f, -1000.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1000.0f, 1000.0f, 1000.0f));
		Sphere* sphere = new Sphere(model, { 0.5f, 0.5f, 0.5f }, lamertian);
		spheres.push_back(sphere);
	}

	{
		glm::mat4x4 model = glm::translate(identity, glm::vec3(0.0f, 1.0f, 0.0f));
		Sphere* sphere = new Sphere(model, { 1.0f, 1.0f, 1.0f }, dielectric, 0.0f, 1.5f);
		spheres.push_back(sphere);
	}

	{
		glm::mat4x4 model = glm::translate(identity, glm::vec3(-4.0f, 1.0f, 0.0f));
		Sphere* sphere = new Sphere(model, { 0.4f, 0.2f, 0.1f }, lamertian);
		spheres.push_back(sphere);
	}

	{
		glm::mat4x4 model = glm::translate(identity, glm::vec3(4.0f, 1.0f, 0.0f));
		Sphere* sphere = new Sphere(model, { 0.7f, 0.6f, 0.5f }, metal);
		spheres.push_back(sphere);
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
				Sphere* sphere = new Sphere(model, { rand01()*rand01(), rand01()*rand01(), rand01()*rand01() }, lamertian);
				spheres.push_back(sphere);
			}
			else if (choose_mat < 0.90f)
			{
				Sphere* sphere = new Sphere(model, { 0.5f*(1.0f + rand01()), 0.5f*(1.0f + rand01()),0.5f*(1.0f + rand01()) }, metal);
				spheres.push_back(sphere);
			}
			else
			{
				Sphere* sphere = new Sphere(model, { 1.0f, 1.0f, 1.0f }, dielectric, 0.0f, 1.5f);
				spheres.push_back(sphere);
			}

		}



	Image target(view_width, view_height);

	PathTracer pt;
	pt.set_target(&target);
	
	for (size_t i = 0; i < spheres.size(); i++)
		pt.add_geometry(spheres[i]);

	pt.set_camera({ 15.0f, 3.0f, 3.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 20.0f, 0.2f, 12.0f);
	pt.trace(100);

	float* hbuffer = (float*)malloc(view_width * view_height * sizeof(float) * 4);
	target.to_host(hbuffer);
	FILE* fp = fopen("rt_weekend.raw", "wb");
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

	for (size_t i = 0; i < spheres.size(); i++)
		delete spheres[i];

	system("pause");

	return 0;
}

