cmake_minimum_required(VERSION 2.6)

project(opendf)

set(CMAKE_MODULE_PATH "${opendf_SOURCE_DIR}/cmake")

include(CheckCXXCompilerFlag)

find_package(OpenSceneGraph REQUIRED osgDB osgViewer osgGA)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

check_cxx_compiler_flag(-std=c++11 HAVE_STD_CXX11)
if(HAVE_STD_CXX11)
    set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
endif()

add_executable(opendf main.cpp)
set_property(TARGET opendf APPEND PROPERTY INCLUDE_DIRECTORIES
    ${OPENSCENEGRAPH_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
)
target_link_libraries(opendf
    ${OPENSCENEGRAPH_LIBRARIES}
    ${SDL2_LIBRARY}
    ${OPENGL_gl_LIBRARY}
)

install(TARGETS opendf RUNTIME DESTINATION bin)
