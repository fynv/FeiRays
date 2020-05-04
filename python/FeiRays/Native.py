import os
import sys
from cffi import FFI

ffi  = FFI()

ffi.cdef("""    
    void* n_vec3_create(float x, float y, float z);
    void n_vec3_destroy(void* cptr);
    void* n_transform_create();
    void n_transform_destroy(void* cptr);
    void n_transform_translate(void* ptr_trans, void* ptr_vec);
    void n_transform_rotate(void* ptr_trans, float angle, void* ptr_vec);
    void n_transform_scale(void* ptr_trans, void* ptr_vec);
    void* n_scene_create(int width, int height);
    void n_scene_destroy(void* cptr);
    int n_scene_width(void* cptr);
    int n_scene_height(void* cptr);
    void n_scene_get_image(void* cptr, float boost, void* data_out);
    void n_scene_set_gradient_sky(void* ptr_scene, void* ptr_color0, void* ptr_color1);
    void n_scene_set_textured_sky(void* ptr_scene, const char* fn_tex, unsigned tex_srgb, float angle, void* ptr_vec);
    void n_scene_add_colored_sphere(void* ptr_scene, void* ptr_trans, int mat_type, void* ptr_mat_color, float mat_fuzz, float mat_ref_idx, float mat_density);
    void n_scene_add_colored_cube(void* ptr_scene, void* ptr_trans, int mat_type, void* ptr_mat_color, float mat_fuzz, float mat_ref_idx, float mat_density);
    void n_scene_add_textured_sphere(void* ptr_scene, void* ptr_trans, const char* fn_tex, unsigned tex_srgb, void* ptr_color);
    void n_scene_add_wavefront_object(void* ptr_scene, void* ptr_trans, const char* path, const char* fn);
    void n_scene_add_sphere_light(void* ptr_scene, void* ptr_center, float radius, void* ptr_color);
    void n_scene_add_sunlight(void* ptr_scene, void* ptr_dir, float angle, void* ptr_color);
    void n_scene_set_camera(void* ptr_scene, void* ptr_lookfrom, void* ptr_lookat, void* ptr_vup, float vfov, float aperture, float focus_dist);
    void n_scene_trace(void* ptr_scene, int num_iter, int interval);
""")

if os.name == 'nt':
    fn_feirays = 'PyFeiRays.dll'
elif os.name == "posix":
    fn_feirays = 'libPyFeiRays.so'

path_feirays = sys.prefix+"/Fei/"+fn_feirays
if not os.path.isfile(path_feirays):
    path_feirays = os.path.dirname(__file__)+"/../"+fn_feirays

native = ffi.dlopen(path_feirays)


