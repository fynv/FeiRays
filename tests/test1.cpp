#include "volk.h"

#include "PathTracer.h"
#include "ColoredIndexedTriangleList.h"
#include "ColoredUnitSphere.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>


typedef ColoredIndexedTriangleList TriangleMesh;
typedef ColoredUnitSphere Sphere;


int main()
{
	const int view_width = 800;
	const int view_height = 400;

	glm::mat4x4 identity = glm::identity<glm::mat4x4>();

	glm::mat4x4 model1 = glm::translate(identity, glm::vec3(0.0f, 0.0f, -1.0f));
	model1 = glm::scale(model1, glm::vec3(0.5f, 0.5f, 0.5f));
	Sphere sphere1(model1, { 0.1f, 0.2f, 0.5f }, lamertian);

	glm::mat4x4 model2 = glm::translate(identity, glm::vec3(0.0f, -100.5f, -1.0f));
	model2 = glm::scale(model2, glm::vec3(100.0f, 100.0f, 100.0f));
	Sphere sphere2(model2, { 0.8f, 0.8f, 0.0f }, lamertian);

	glm::mat4x4 model3 = glm::translate(identity, glm::vec3(1.0f, 0.0f, -1.0f));
	model3 = glm::scale(model3, glm::vec3(0.5f, 0.5f, 0.5f));
	Sphere sphere3(model3, { 0.8f, 0.6f, 0.2f }, metal, 0.3f);

	glm::mat4x4 model4 = glm::translate(identity, glm::vec3(-1.0f, 0.0f, -1.0f));
	model4 = glm::scale(model4, glm::vec3(0.5f, 0.5f, 0.5f));
	Sphere sphere4(model4, { 1.0f, 1.0f, 1.0f }, dielectric, 0.0f, 1.5f);

	glm::mat4x4 model5 = glm::translate(identity, glm::vec3(-1.0f, 0.0f, -1.0f));
	model5 = glm::scale(model5, glm::vec3(0.45f, 0.45f, 0.45f));
	Sphere sphere5(model5, { 1.0f, 1.0f, 1.0f }, dielectric, 0.0f, 1.0/1.5f);

	Image target(view_width, view_height);

	PathTracer pt;
	pt.set_target(&target);
	pt.add_geometry(&sphere1);
	pt.add_geometry(&sphere2);
	pt.add_geometry(&sphere3);
	pt.add_geometry(&sphere4);
	pt.add_geometry(&sphere5);
	
	pt.set_camera({ 3.0f, 3.0f, 2.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, 20.0f, 0.5f, 5.2f);

	pt.trace(100);

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
