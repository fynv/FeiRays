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
#include "UnitSphereCheckerTex.h"

int main()
{
	const int view_width = 900;
	const int view_height = 600;

	Image target(view_width, view_height);

	PathTracer pt;
	pt.set_target(&target);

	glm::mat4x4 identity = glm::identity<glm::mat4x4>();

	glm::mat4x4 model0 = glm::translate(identity, glm::vec3(0.0f, -1000.0f, 0.0f));
	model0 = glm::scale(model0, glm::vec3(1000.0f, 1000.0f, 1000.0f));
	UnitSphereCheckerTex sphere0(model0, 0.3f, { 0.2f, 0.3f, 0.1f }, { 0.9f, 0.9f, 0.9f });
	pt.add_geometry(&sphere0);

	glm::mat4x4 model1 = glm::translate(identity, glm::vec3(0.0f, 1.0f, 3.0f));
	ColoredUnitSphere sphere1(model1, { 1.0f, 1.0f, 1.0f }, dielectric, 0.0f, 1.5f);
	pt.add_geometry(&sphere1);

	glm::mat4x4 model2 = glm::translate(identity, glm::vec3(-3.0f, 1.0f, -1.5f));
	ColoredUnitSphere sphere2(model2, { 0.7f, 0.6f, 0.5f }, metal);
	pt.add_geometry(&sphere2);

	LambertianObject obj(pt, "../data/Medieval_building", "Medieval_building.obj", identity);
	pt.add_geometry(obj.get_geo());

	pt.set_camera({ -12.0f, 6.0f, 12.0f }, { 0.0f, 1.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 20.0f, 0.2f, 16.0f);
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


