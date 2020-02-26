cmake_minimum_required (VERSION 3.0)

project(PyFeiRays)
find_package(PythonLibs 3 REQUIRED)

include_directories(${PYTHON_INCLUDE_DIRS})

add_library(PyFeiRays SHARED dds_reader.hpp Scene.cpp Scene.h texture_map.cpp texture_map.h pyFeiRays.cpp)
target_link_libraries(PyFeiRays ${PYTHON_LIBRARIES} FeiRays volk)

if (WIN32) 
set_target_properties(PyFeiRays PROPERTIES SUFFIX ".pyd")
else()
set_target_properties(PyFeiRays PROPERTIES SUFFIX ".so")
set_target_properties(PyFeiRays PROPERTIES PREFIX "")
endif()



