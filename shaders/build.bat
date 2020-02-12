glslangValidator -V final.comp -o final.spv

glslangValidator -V raygen.rgen -o raygen.spv
glslangValidator -V miss.rmiss -o miss.spv
glslangValidator -V miss_shadow.rmiss -o miss_shadow.spv
glslangValidator -V closesthit_colored_indexed_triangle_lists.rchit -o closesthit_colored_indexed_triangle_lists.spv
glslangValidator -V intersection_unit_spheres.rint -o intersection_unit_spheres.spv
glslangValidator -V closesthit_colored_unit_spheres.rchit -o closesthit_colored_unit_spheres.spv
glslangValidator -V closesthit_unit_spheres_checker_tex.rchit -o closesthit_unit_spheres_checker_tex.spv

