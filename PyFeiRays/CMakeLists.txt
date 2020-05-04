cmake_minimum_required (VERSION 3.0)

project(PyFeiRays)

add_library(PyFeiRays SHARED dds_reader.hpp Scene.cpp Scene.h texture_map.cpp texture_map.h api.h api_utils.cpp api_scene.cpp)
target_link_libraries(PyFeiRays FeiRays volk)

