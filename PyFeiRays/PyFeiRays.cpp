#include <Python.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#define _USE_MATH_DEFINES
#include <math.h>
#include "Scene.h"

static PyObject* n_transform_create(PyObject* self, PyObject* args)
{
	glm::mat4x4* trans = new glm::mat4x4;
	*trans = glm::identity<glm::mat4x4>();
	return PyLong_FromVoidPtr(trans);
}

static PyObject* n_transform_destroy(PyObject* self, PyObject* args)
{
	glm::mat4x4* trans = (glm::mat4x4*)PyLong_AsVoidPtr(PyTuple_GetItem(args, 0));
	delete trans;
	return PyLong_FromLong(0);
}

static glm::vec3 PyTuple_As_Vec3(PyObject* tuple)
{
	glm::vec3 ret;
	ret.x = (float)PyFloat_AsDouble(PyTuple_GetItem(tuple, 0));
	ret.y = (float)PyFloat_AsDouble(PyTuple_GetItem(tuple, 1));
	ret.z = (float)PyFloat_AsDouble(PyTuple_GetItem(tuple, 2));
	return ret;
}

static PyObject* n_transform_translate(PyObject* self, PyObject* args)
{
	glm::mat4x4* trans = (glm::mat4x4*)PyLong_AsVoidPtr(PyTuple_GetItem(args, 0));
	glm::vec3 vec = PyTuple_As_Vec3(PyTuple_GetItem(args, 1));
	*trans = glm::translate(*trans, vec);
	return PyLong_FromLong(0);
}

static PyObject* n_transform_rotate(PyObject* self, PyObject* args)
{
	glm::mat4x4* trans = (glm::mat4x4*)PyLong_AsVoidPtr(PyTuple_GetItem(args, 0));
	float angle = (float)PyFloat_AsDouble(PyTuple_GetItem(args, 1));
	float rad = angle / 180.0f*(float)M_PI;
	glm::vec3 vec = PyTuple_As_Vec3(PyTuple_GetItem(args, 2));
	*trans = glm::rotate(*trans, rad, vec);
	return PyLong_FromLong(0);
}

static PyObject* n_transform_scale(PyObject* self, PyObject* args)
{
	glm::mat4x4* trans = (glm::mat4x4*)PyLong_AsVoidPtr(PyTuple_GetItem(args, 0));
	glm::vec3 vec = PyTuple_As_Vec3(PyTuple_GetItem(args, 1));
	*trans = glm::scale(*trans, vec);
	return PyLong_FromLong(0);
}

static PyObject* n_transform_to_python(PyObject* self, PyObject* args)
{
	glm::mat4x4* trans = (glm::mat4x4*)PyLong_AsVoidPtr(PyTuple_GetItem(args, 0));
	PyObject* mat = PyList_New(4);
	for (int i = 0; i < 4; i++)
	{
		PyObject* col = PyList_New(4);
		for (int j = 0; j < 4; j++)
		{
			PyObject* num = PyFloat_FromDouble((double)(*trans)[i][j]);
			PyList_SetItem(col, j, num);
		}
		PyList_SetItem(mat, i, col);
	}
	return mat;
}

static PyObject* n_scene_create(PyObject* self, PyObject* args)
{
	int width = (int)PyLong_AsLong(PyTuple_GetItem(args, 0));
	int height = (int)PyLong_AsLong(PyTuple_GetItem(args, 1));
	Scene* scene = new Scene(width, height);
	return PyLong_FromVoidPtr(scene);
}


static PyObject* n_scene_destroy(PyObject* self, PyObject* args)
{
	Scene* scene = (Scene*)PyLong_AsVoidPtr(PyTuple_GetItem(args, 0));
	delete scene;
	return PyLong_FromLong(0);
}

static PyObject* n_scene_size(PyObject* self, PyObject* args)
{
	Scene* scene = (Scene*)PyLong_AsVoidPtr(PyTuple_GetItem(args, 0));
	PyObject* ret = PyTuple_New(2);
	PyTuple_SetItem(ret, 0, PyLong_FromLong((long)scene->image().width()));
	PyTuple_SetItem(ret, 1, PyLong_FromLong((long)scene->image().height()));
	return ret;
}

static PyObject* n_scene_get_image(PyObject* self, PyObject* args)
{
	Scene* scene = (Scene*)PyLong_AsVoidPtr(PyTuple_GetItem(args, 0));
	float boost = (float)PyFloat_AsDouble(PyTuple_GetItem(args, 1));

	const Image& image = scene->image();
	int width = image.width();
	int height = image.height();

	PyObject* dataBuf = PyBytes_FromStringAndSize(nullptr, 3 * width * height);
	char* p;
	ssize_t size;
	PyBytes_AsStringAndSize(dataBuf, &p, &size);
	image.to_host_srgb((unsigned char*)p, boost);
	return dataBuf;
}

static PyObject* n_scene_set_gradient_sky(PyObject* self, PyObject* args)
{
	Scene* scene = (Scene*)PyLong_AsVoidPtr(PyTuple_GetItem(args, 0));
	glm::vec3 color0 = PyTuple_As_Vec3(PyTuple_GetItem(args, 1));
	glm::vec3 color1 = PyTuple_As_Vec3(PyTuple_GetItem(args, 2));
	scene->set_gradient_sky(color0, color1);
	return PyLong_FromLong(0);
}

static PyObject* n_scene_set_textured_sky(PyObject* self, PyObject* args)
{
	Scene* scene = (Scene*)PyLong_AsVoidPtr(PyTuple_GetItem(args, 0));
	const char* fn_tex = PyUnicode_AsUTF8(PyTuple_GetItem(args, 1));
	bool srgb = PyObject_IsTrue(PyTuple_GetItem(args, 2));
	PyObject* py_angle = PyTuple_GetItem(args, 3);
	if (py_angle != Py_None)
	{
		float angle = (float)PyFloat_AsDouble(py_angle);
		float rad = angle / 180.0f*(float)M_PI;
		glm::vec3 v = PyTuple_As_Vec3(PyTuple_GetItem(args, 4));
		scene->set_textured_sky(fn_tex, srgb, rad, v);
	}
	else
	{
		scene->set_textured_sky(fn_tex, srgb);
	}
	return PyLong_FromLong(0);
}

static PyObject* n_add_colored_sphere(PyObject* self, PyObject* args)
{
	Scene* scene = (Scene*)PyLong_AsVoidPtr(PyTuple_GetItem(args, 0));
	glm::mat4x4* trans = (glm::mat4x4*)PyLong_AsVoidPtr(PyTuple_GetItem(args, 1));
	Material mat;
	{
		PyObject* obj = PyTuple_GetItem(args, 2);
		if (obj != Py_None)	mat.type = (MaterialType)PyLong_AsLong(obj);
	}
	{
		PyObject* obj = PyTuple_GetItem(args, 3);
		if (obj != Py_None)	mat.color = PyTuple_As_Vec3(obj);
	}
	{
		PyObject* obj = PyTuple_GetItem(args, 4);
		if (obj != Py_None)	mat.fuzz = (float)PyFloat_AsDouble(obj);
	}
	{
		PyObject* obj = PyTuple_GetItem(args, 5);
		if (obj != Py_None)	mat.ref_idx = (float)PyFloat_AsDouble(obj);
	}
	{
		PyObject* obj = PyTuple_GetItem(args, 6);
		if (obj != Py_None)	mat.density = (float)PyFloat_AsDouble(obj);
	}
	scene->add_colored_sphere(*trans, mat);
	return PyLong_FromLong(0);
}


static PyObject* n_add_colored_cube(PyObject* self, PyObject* args)
{
	Scene* scene = (Scene*)PyLong_AsVoidPtr(PyTuple_GetItem(args, 0));
	glm::mat4x4* trans = (glm::mat4x4*)PyLong_AsVoidPtr(PyTuple_GetItem(args, 1));
	Material mat;
	{
		PyObject* obj = PyTuple_GetItem(args, 2);
		if (obj != Py_None)	mat.type = (MaterialType)PyLong_AsLong(obj);
	}
	{
		PyObject* obj = PyTuple_GetItem(args, 3);
		if (obj != Py_None)	mat.color = PyTuple_As_Vec3(obj);
	}
	{
		PyObject* obj = PyTuple_GetItem(args, 4);
		if (obj != Py_None)	mat.fuzz = (float)PyFloat_AsDouble(obj);
	}
	{
		PyObject* obj = PyTuple_GetItem(args, 5);
		if (obj != Py_None)	mat.ref_idx = (float)PyFloat_AsDouble(obj);
	}
	{
		PyObject* obj = PyTuple_GetItem(args, 6);
		if (obj != Py_None)	mat.density = (float)PyFloat_AsDouble(obj);
	}
	scene->add_colored_cube(*trans, mat);
	return PyLong_FromLong(0);
}

static PyObject* n_add_textured_sphere(PyObject* self, PyObject* args)
{
	Scene* scene = (Scene*)PyLong_AsVoidPtr(PyTuple_GetItem(args, 0));
	glm::mat4x4* trans = (glm::mat4x4*)PyLong_AsVoidPtr(PyTuple_GetItem(args, 1));
	const char* fn_tex = PyUnicode_AsUTF8(PyTuple_GetItem(args, 2));
	bool srgb = PyObject_IsTrue(PyTuple_GetItem(args, 3));
	glm::vec3 color = PyTuple_As_Vec3(PyTuple_GetItem(args, 4));
	scene->add_textured_sphere(*trans, fn_tex, srgb, color);
	return PyLong_FromLong(0);
}

static PyObject* n_add_wavefront_object(PyObject* self, PyObject* args)
{
	Scene* scene = (Scene*)PyLong_AsVoidPtr(PyTuple_GetItem(args, 0));
	glm::mat4x4* trans = (glm::mat4x4*)PyLong_AsVoidPtr(PyTuple_GetItem(args, 1));
	const char* path = PyUnicode_AsUTF8(PyTuple_GetItem(args, 2));
	const char* fn = PyUnicode_AsUTF8(PyTuple_GetItem(args, 3));
	scene->add_wavefront_object(*trans, path, fn);
	return PyLong_FromLong(0);
}

static PyObject* n_add_sphere_light(PyObject* self, PyObject* args)
{
	Scene* scene = (Scene*)PyLong_AsVoidPtr(PyTuple_GetItem(args, 0));
	glm::vec3 center = PyTuple_As_Vec3(PyTuple_GetItem(args, 1));
	float radius = (float)PyFloat_AsDouble(PyTuple_GetItem(args, 2));
	glm::vec3 color = PyTuple_As_Vec3(PyTuple_GetItem(args, 3));
	scene->add_sphere_light(center, radius, color);
	return PyLong_FromLong(0);
}

static PyObject* n_add_sunlight(PyObject* self, PyObject* args)
{
	Scene* scene = (Scene*)PyLong_AsVoidPtr(PyTuple_GetItem(args, 0));
	glm::vec3 direction = PyTuple_As_Vec3(PyTuple_GetItem(args, 1));
	float angle = (float)PyFloat_AsDouble(PyTuple_GetItem(args, 2));
	float rad = angle / 180.0f*(float)M_PI;
	glm::vec3 color = PyTuple_As_Vec3(PyTuple_GetItem(args, 3));
	scene->add_sunlight(direction, rad, color);
	return PyLong_FromLong(0);
}


static PyObject* n_scene_set_camera(PyObject* self, PyObject* args)
{
	Scene* scene = (Scene*)PyLong_AsVoidPtr(PyTuple_GetItem(args, 0));
	glm::vec3 lookfrom = PyTuple_As_Vec3(PyTuple_GetItem(args, 1));
	glm::vec3 lookat = PyTuple_As_Vec3(PyTuple_GetItem(args, 2));
	glm::vec3 vup = PyTuple_As_Vec3(PyTuple_GetItem(args, 3));
	float vfov = (float)PyFloat_AsDouble(PyTuple_GetItem(args, 4));
	float aperture = (float)PyFloat_AsDouble(PyTuple_GetItem(args, 5));
	float focus_dist = (float)PyFloat_AsDouble(PyTuple_GetItem(args, 6));
	scene->set_camera(lookfrom, lookat, vup, vfov, aperture, focus_dist);
	return PyLong_FromLong(0);
}

static PyObject* n_scene_trace(PyObject* self, PyObject* args)
{
	Scene* scene = (Scene*)PyLong_AsVoidPtr(PyTuple_GetItem(args, 0));
	int num_iter = (int)PyLong_AsLong(PyTuple_GetItem(args, 1));
	int interval = (int)PyLong_AsLong(PyTuple_GetItem(args, 2));
	scene->trace(num_iter, interval);
	return PyLong_FromLong(0);
}

static PyMethodDef s_Methods[] = {
	{ "n_transform_create", n_transform_create, METH_VARARGS, "" },
	{ "n_transform_destroy", n_transform_destroy, METH_VARARGS, "" },
	{ "n_transform_translate", n_transform_translate, METH_VARARGS, "" },
	{ "n_transform_rotate", n_transform_rotate, METH_VARARGS, "" },
	{ "n_transform_scale", n_transform_scale, METH_VARARGS, "" },
	{ "n_transform_to_python", n_transform_to_python, METH_VARARGS, "" },
	{ "n_scene_create", n_scene_create, METH_VARARGS, "" },
	{ "n_scene_destroy", n_scene_destroy, METH_VARARGS, "" },
	{ "n_scene_size", n_scene_size, METH_VARARGS, "" },
	{ "n_scene_get_image", n_scene_get_image, METH_VARARGS, "" },
	{ "n_scene_set_gradient_sky", n_scene_set_gradient_sky, METH_VARARGS, "" },
	{ "n_scene_set_textured_sky", n_scene_set_textured_sky, METH_VARARGS, "" },
	{ "n_add_colored_sphere", n_add_colored_sphere, METH_VARARGS, "" },
	{ "n_add_colored_cube", n_add_colored_cube, METH_VARARGS, "" },
	{ "n_add_textured_sphere", n_add_textured_sphere, METH_VARARGS, "" },
	{ "n_add_wavefront_object", n_add_wavefront_object, METH_VARARGS, "" },
	{ "n_add_sphere_light", n_add_sphere_light, METH_VARARGS, "" },
	{ "n_add_sunlight", n_add_sunlight, METH_VARARGS, "" },
	{ "n_scene_set_camera", n_scene_set_camera, METH_VARARGS, "" },
	{ "n_scene_trace", n_scene_trace, METH_VARARGS, "" },
	0
};


static struct PyModuleDef cModPyDem = { PyModuleDef_HEAD_INIT, "FeiRays_module", "", -1, s_Methods };

PyMODINIT_FUNC PyInit_PyFeiRays(void)
{
	return PyModule_Create(&cModPyDem);
}

