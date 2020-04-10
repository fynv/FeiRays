cmake_minimum_required (VERSION 3.0)

project(FeiRays)

set (INCLUDE_DIR
thirdparty
thirdparty/volk
thirdparty/Vulkan-Headers/include
thirdparty/glm/glm
.
)

include_directories(${INCLUDE_DIR})

add_subdirectory(thirdparty/volk)

set (SOURCE
context.cpp
RNGInitializer.cpp
PathTracer.cpp
ColoredIndexedTriangleList.cpp
ColoredUnitSphere.cpp
UnitSphereCheckerTex.cpp
TexturedTriangleList.cpp
TexturedUnitSphere.cpp
WavefrontIndexedTriangleList.cpp
)

set (HEADER
Timing.h
context.h
RNGState_xorwow.h
xor_wow_data.hpp
Material.h
RNGInitializer.h
PathTracer.h
ColoredIndexedTriangleList.h
ColoredUnitSphere.h
UnitSphereCheckerTex.h
TexturedTriangleList.h
TexturedUnitSphere.h
WavefrontIndexedTriangleList.h
)

if (WIN32) 
set (DEFINES  ${DEFINES}
-D"_CRT_SECURE_NO_DEPRECATE"  
-D"_SCL_SECURE_NO_DEPRECATE" 
-D"_CRT_SECURE_NO_WARNINGS"
)
else()
add_definitions(-std=c++14)
add_compile_options(-fPIC)
endif()

add_definitions(${DEFINES})

add_library(FeiRays ${SOURCE} ${HEADER})

add_executable(test_rng tests/test_rng.cpp)
target_link_libraries(test_rng FeiRays volk)

add_executable(rt_weekend tests/rt_weekend.cpp)
target_link_libraries(rt_weekend FeiRays volk)

add_executable(test1 tests/test1.cpp)
target_link_libraries(test1 FeiRays volk)

add_executable(test2 tests/test2.cpp)
target_link_libraries(test2 FeiRays volk)

add_executable(test3 tests/test3.cpp tests/dds_reader.hpp tests/texture_map.cpp tests/texture_map.h tests/lambertian_obj.cpp tests/lambertian_obj.h)
target_link_libraries(test3 FeiRays volk)

add_executable(test4 tests/test4.cpp)
target_link_libraries(test4 FeiRays volk)

add_executable(test5 tests/test5.cpp)
target_link_libraries(test5 FeiRays volk)

add_executable(test6 tests/test6.cpp)
target_link_libraries(test6 FeiRays volk)

add_executable(test7 tests/test7.cpp tests/dds_reader.hpp tests/texture_map.cpp tests/texture_map.h tests/wavefront_obj.cpp tests/wavefront_obj.h)
target_link_libraries(test7 FeiRays volk)

set(BUILD_PYTHON_EXTENSION false CACHE BOOL "Build Python Extension")

if (BUILD_PYTHON_EXTENSION)
add_subdirectory(PyFeiRays)
endif()
