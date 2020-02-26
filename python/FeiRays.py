import PyFeiRays as native
from PIL import Image as pil_Image

class Transform:
    def __init__ (self):
        self.m_cptr = native.n_transform_create()

    def __del__ (self):
        native.n_transform_destroy(self.m_cptr)

    def to_list(self):
        return native.n_transform_to_python(self.m_cptr)

    def __str__(self):
        return str(self.to_list())

    def translate(self, x, y, z):
        native.n_transform_translate(self.m_cptr, (x,y,z))

    def rotate(self, angle, axis):
        native.n_transform_rotate(self.m_cptr, angle, axis)

    def scale(self, x, y, z):
        native.n_transform_scale(self.m_cptr, (x,y,z))

# material types
LAMBERTIAN = 0
METAL = 1
DIELECTRIC = 2
EMISSIVE = 3
FOGGY = 4

class Scene:
    def __init__ (self, width, height):
        self.m_cptr = native.n_scene_create(width, height)

    def __del__ (self):
        native.n_scene_destroy(self.m_cptr)

    def size(self):
        return native.n_scene_size(self.m_cptr)

    def get_image(self, boost = 1.0):
        data = native.n_scene_get_image(self.m_cptr, boost)
        return pil_Image.frombytes('RGB', self.size(), data)

    def set_gradient_sky(self, color0 = (1.0, 1.0, 1.0), color1=(0.5, 0.7, 1.0)):
        native.n_scene_set_gradient_sky(self.m_cptr, color0, color1)

    def set_textured_sky(self, filename, srgb = True, angle = None, axis = None):
        native.n_scene_set_textured_sky(self.m_cptr, filename, srgb, angle, axis)

    def add_colored_sphere(self, transform, material_type = LAMBERTIAN, color = (1.0, 1.0, 1.0), fuzz = None, ref_idx = None, density = None):
        native.n_add_colored_sphere(self.m_cptr, transform.m_cptr, material_type, color, fuzz, ref_idx, density)

    def add_colored_cube(self, transform, material_type = LAMBERTIAN, color = (1.0, 1.0, 1.0), fuzz = None, ref_idx = None, density = None):
        native.n_add_colored_cube(self.m_cptr, transform.m_cptr, material_type, color, fuzz, ref_idx, density)

    def add_textured_sphere(self, transform, filename, srgb=True, color=(1.0, 1.0, 1.0)):
        native.n_add_textured_sphere(self.m_cptr, transform.m_cptr, filename, srgb, color)

    def add_wavefront_object(self, transform, path, filename):
        native.n_add_wavefront_object(self.m_cptr, transform.m_cptr, path, filename)

    def add_sphere_light(self, center, radius, color):
        native.n_add_sphere_light(self.m_cptr, center, radius, color)

    def add_sunlight(self, direction, angle, color):
        native.n_add_sunlight(self.m_cptr, direction, angle, color)

    def set_camera(self, look_from, lookat, vup, vfov, aperture = 0.0, focus_dist = 1.0):
        native.n_scene_set_camera(self.m_cptr, look_from, lookat, vup, vfov, aperture, focus_dist)

    def trace(self, num_iter = 100, interval = -1):
        native.n_scene_trace(self.m_cptr, num_iter, interval)
