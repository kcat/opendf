cmake_minimum_required(VERSION 2.6)

project(opendf)

set(CMAKE_MODULE_PATH "${opendf_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")

include(CheckCXXCompilerFlag)

check_cxx_compiler_flag(-std=c++11 HAVE_STD_CXX11)
if(HAVE_STD_CXX11)
    set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
endif()

check_cxx_compiler_flag("-Wall" HAVE_WALL_SWITCH)
if(HAVE_WALL_SWITCH)
    add_definitions("-Wall")
endif()

find_package(OpenSceneGraph REQUIRED osgDB osgViewer osgGA)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(MyGUI REQUIRED)

include_directories("${opendf_SOURCE_DIR}")

set(SRCS components/sdlutil/graphicswindow.cpp
         components/mygui_osg/rendermanager.cpp
         components/mygui_osg/texture.cpp
         components/mygui_osg/vertexbuffer.cpp
         src/engine.cpp
         main.cpp
)
set(HDRS components/sdlutil/graphicswindow.hpp
         components/mygui_osg/diagnostic.h
         components/mygui_osg/rendermanager.h
         components/mygui_osg/texture.h
         components/mygui_osg/vertexbuffer.h
         src/engine.hpp
)

add_executable(opendf ${SRCS} ${HDRS})
set_property(TARGET opendf APPEND PROPERTY INCLUDE_DIRECTORIES
    "${opendf_SOURCE_DIR}/src"
    ${OPENSCENEGRAPH_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${MYGUI_INCLUDE_DIRS}
)
target_link_libraries(opendf
    ${OPENSCENEGRAPH_LIBRARIES}
    ${SDL2_LIBRARY}
    ${OPENGL_gl_LIBRARY}
    ${MYGUI_LIBRARIES}
)

install(TARGETS opendf RUNTIME DESTINATION bin)
