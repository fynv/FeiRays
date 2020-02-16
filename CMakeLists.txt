cmake_minimum_required (VERSION 3.0)

project(FeiRays)

find_package(CUDA REQUIRED)

set (INCLUDE_DIR
thirdparty
thirdparty/volk
thirdparty/Vulkan-Headers/include
thirdparty/glm/glm
.
)

include_directories(${INCLUDE_DIR})

if (WIN32) 
add_definitions(-D"VK_USE_PLATFORM_WIN32_KHR")
endif()

add_subdirectory(thirdparty/volk)

set (SOURCE
context.cpp
rand_state_init.cu
PathTracer.cpp
ColoredIndexedTriangleList.cpp
ColoredUnitSphere.cpp
UnitSphereCheckerTex.cpp
TexturedTriangleList.cpp
TexturedUnitSphere.cpp
)

set (HEADER
Timing.h
context.h
RNGState.h
xor_wow_data.hpp
rand_state_init.hpp
PathTracer.h
ColoredIndexedTriangleList.h
ColoredUnitSphere.h
UnitSphereCheckerTex.h
TexturedTriangleList.h
TexturedUnitSphere.h
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

cuda_add_library(FeiRays ${SOURCE} ${HEADER})

add_executable(rt_weekend tests/rt_weekend.cpp)
target_link_libraries(rt_weekend FeiRays volk)

add_executable(test1 tests/test1.cpp)
target_link_libraries(test1 FeiRays volk)

add_executable(test2 tests/test2.cpp)
target_link_libraries(test2 FeiRays volk)

add_executable(test3 tests/test3.cpp tests/dds_reader.hpp tests/lambertian_obj.cpp tests/lambertian_obj.h)
target_link_libraries(test3 FeiRays volk)

add_executable(test4 tests/test4.cpp)
target_link_libraries(test4 FeiRays volk)
