#!/bin/sh
glslangValidator -V common/rand_init.comp -o common/rand_init.spv --target-env vulkan1.2
glslangValidator -V common/vert_srgb.vert -o common/vert_srgb.spv --target-env vulkan1.2
glslangValidator -V common/frag_srgb.vert -o common/frag_srgb.spv --target-env vulkan1.2

glslangValidator -V common/miss.rmiss -o common/miss.spv --target-env vulkan1.2
glslangValidator -V common/miss_tex_sky.rmiss -o common/miss_tex_sky.spv --target-env vulkan1.2

glslangValidator -V geometry/closesthit_colored_indexed_triangle_lists.rchit -o geometry/closesthit_colored_indexed_triangle_lists.spv --target-env vulkan1.2
glslangValidator -V geometry/intersection_unit_spheres.rint -o geometry/intersection_unit_spheres.spv --target-env vulkan1.2
glslangValidator -V geometry/closesthit_colored_unit_spheres.rchit -o geometry/closesthit_colored_unit_spheres.spv --target-env vulkan1.2
glslangValidator -V geometry/closesthit_unit_spheres_checker_tex.rchit -o geometry/closesthit_unit_spheres_checker_tex.spv --target-env vulkan1.2
glslangValidator -V geometry/closesthit_textured_triangle_lists.rchit -o geometry/closesthit_textured_triangle_lists.spv --target-env vulkan1.2
glslangValidator -V geometry/closesthit_textured_unit_spheres.rchit -o geometry/closesthit_textured_unit_spheres.spv --target-env vulkan1.2
glslangValidator -V geometry/closesthit_sphere_lights.rchit -o geometry/closesthit_sphere_lights.spv --target-env vulkan1.2
glslangValidator -V geometry/closesthit_wavefront_indexed_triangle_lists.rchit -o geometry/closesthit_wavefront_indexed_triangle_lists.spv --target-env vulkan1.2

glslangValidator -V geometry/closesthit_ply_mesh.rchit -o geometry/closesthit_ply_mesh.spv --target-env vulkan1.2


glslangValidator -V path_tracer/raygen.rgen -o path_tracer/raygen.spv --target-env vulkan1.2
glslangValidator -V path_tracer/final.comp -o path_tracer/final.spv --target-env vulkan1.2



