from .Native import native
from .utils import Vec3, Transform
from PIL import Image as pil_Image

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
        width = native.n_scene_width(self.m_cptr)
        height = native.n_scene_height(self.m_cptr)
        return (width, height)

    def get_image(self, boost = 1.0):
        width = native.n_scene_width(self.m_cptr)
        height = native.n_scene_height(self.m_cptr)
        data = b'\x00'*(width*height*3);
        native.n_scene_get_image(self.m_cptr, boost, data)
        return pil_Image.frombytes('RGB', (width, height), data)

    def set_gradient_sky(self, color0 = (1.0, 1.0, 1.0), color1=(0.5, 0.7, 1.0)):
        v_color0 = Vec3(color0)
        v_color1 = Vec3(color1)
        native.n_scene_set_gradient_sky(self.m_cptr, v_color0.m_cptr, v_color1.m_cptr)

    def set_textured_sky(self, filename, srgb = True, angle = 0.0, axis = None):
        p_axis = 0
        if axis!=None:
            v_axis = Vec3(axis)
            p_axis = v_axis.m_cptr
        native.n_scene_set_textured_sky(self.m_cptr, filename.encode('utf-8'), srgb, f_angle, p_axis)

    def add_colored_sphere(self, transform, material_type = LAMBERTIAN, color = (1.0, 1.0, 1.0), fuzz = 0.0, ref_idx = 0.0, density = 0.0):
        v_color = Vec3(color)
        native.n_scene_add_colored_sphere(self.m_cptr, transform.m_cptr, material_type, v_color.m_cptr, fuzz, ref_idx, density)

    def add_colored_cube(self, transform, material_type = LAMBERTIAN, color = (1.0, 1.0, 1.0), fuzz = 0.0, ref_idx = 0.0, density = 0.0):
        v_color = Vec3(color)
        native.n_scene_add_colored_cube(self.m_cptr, transform.m_cptr, material_type, v_color.m_cptr, fuzz, ref_idx, density)

    def add_textured_sphere(self, transform, filename, srgb=True, color=(1.0, 1.0, 1.0)):
        v_color = Vec3(color)
        native.n_scene_add_textured_sphere(self.m_cptr, transform.m_cptr, filename.encode('utf-8'), srgb, v_color.m_cptr)

    def add_wavefront_object(self, transform, path, filename):
        native.n_scene_add_wavefront_object(self.m_cptr, transform.m_cptr, path.encode('utf-8'), filename.encode('utf-8'))

    def add_sphere_light(self, center, radius, color):
        v_center = Vec3(center)
        v_color = Vec3(color)
        native.n_scene_add_sphere_light(self.m_cptr, v_center.m_cptr, radius, v_color.m_cptr)

    def add_sunlight(self, direction, angle, color):
        v_dir = Vec3(direction)
        v_color = Vec3(color)
        native.n_scene_add_sunlight(self.m_cptr, v_dir.m_cptr, angle, v_color.m_cptr)

    def set_camera(self, look_from, lookat, vup, vfov, aperture = 0.0, focus_dist = 1.0):
        v_look_from = Vec3(look_from)
        v_look_at = Vec3(lookat)
        v_vup =  Vec3(vup)
        native.n_scene_set_camera(self.m_cptr, v_look_from.m_cptr, v_look_at.m_cptr, v_vup.m_cptr, vfov, aperture, focus_dist)

    def trace(self, num_iter = 100, interval = -1):
        native.n_scene_trace(self.m_cptr, num_iter, interval)
