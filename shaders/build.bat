glslangValidator -V common/rand_init.comp -o common/rand_init.spv
glslangValidator -V common/vert_srgb.vert -o common/vert_srgb.spv
glslangValidator -V common/frag_srgb.frag -o common/frag_srgb.spv

glslangValidator -V common/miss.rmiss -o common/miss.spv
glslangValidator -V common/miss_tex_sky.rmiss -o common/miss_tex_sky.spv


glslangValidator -V geometry/closesthit_colored_indexed_triangle_lists.rchit -o geometry/closesthit_colored_indexed_triangle_lists.spv
glslangValidator -V geometry/intersection_unit_spheres.rint -o geometry/intersection_unit_spheres.spv
glslangValidator -V geometry/closesthit_colored_unit_spheres.rchit -o geometry/closesthit_colored_unit_spheres.spv
glslangValidator -V geometry/closesthit_unit_spheres_checker_tex.rchit -o geometry/closesthit_unit_spheres_checker_tex.spv
glslangValidator -V geometry/closesthit_textured_triangle_lists.rchit -o geometry/closesthit_textured_triangle_lists.spv
glslangValidator -V geometry/closesthit_textured_unit_spheres.rchit -o geometry/closesthit_textured_unit_spheres.spv
glslangValidator -V geometry/closesthit_sphere_lights.rchit -o geometry/closesthit_sphere_lights.spv
glslangValidator -V geometry/closesthit_wavefront_indexed_triangle_lists.rchit -o geometry/closesthit_wavefront_indexed_triangle_lists.spv


glslangValidator -V path_tracer/raygen.rgen -o path_tracer/raygen.spv
glslangValidator -V path_tracer/final.comp -o path_tracer/final.spv



