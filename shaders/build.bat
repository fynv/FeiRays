glslangValidator -V final.comp -o final.spv

glslangValidator -V raygen.rgen -o raygen.spv
glslangValidator -V miss.rmiss -o miss.spv
glslangValidator -V miss_shadow.rmiss -o miss_shadow.spv
glslangValidator -V closesthit_triangles.rchit -o closesthit_triangles.spv
glslangValidator -V intersection_spheres.rint -o intersection_spheres.spv
glslangValidator -V closesthit_spheres.rchit -o closesthit_spheres.spv


