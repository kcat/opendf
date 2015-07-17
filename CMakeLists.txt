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

find_package(OpenSceneGraph REQUIRED osgDB osgViewer osgGA osgUtil)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(MyGUI REQUIRED)

include_directories("${opendf_SOURCE_DIR}")

set(SRCS components/sdlutil/graphicswindow.cpp
         components/settings/configfile.cpp
         components/vfs/manager.cpp
         components/vfs/osg_callbacks.cpp
         components/resource/texturemanager.cpp
         components/resource/meshmanager.cpp
         components/mygui_osg/rendermanager.cpp
         components/mygui_osg/texture.cpp
         components/mygui_osg/vertexbuffer.cpp
         components/mygui_osg/datamanager.cpp
         components/dfosg/texloader.cpp
         components/dfosg/meshloader.cpp
         src/input/input.cpp
         src/gui/gui.cpp
         src/world/world.cpp
         src/world/pitems.cpp
         src/world/ditems.cpp
         src/world/dblocks.cpp
         src/log.cpp
         src/cvars.cpp
         src/engine.cpp
         main.cpp
)
set(HDRS components/sdlutil/graphicswindow.hpp
         components/settings/configfile.hpp
         components/vfs/manager.hpp
         components/vfs/osg_callbacks.hpp
         components/resource/texturemanager.hpp
         components/resource/meshmanager.hpp
         components/mygui_osg/diagnostic.h
         components/mygui_osg/rendermanager.h
         components/mygui_osg/texture.h
         components/mygui_osg/vertexbuffer.h
         components/mygui_osg/datamanager.h
         components/dfosg/texloader.hpp
         components/dfosg/meshloader.hpp
         src/input/input.hpp
         src/gui/iface.hpp
         src/gui/gui.hpp
         src/world/iface.hpp
         src/world/world.hpp
         src/world/itembase.hpp
         src/world/pitems.hpp
         src/world/ditems.hpp
         src/world/dblocks.hpp
         src/log.hpp
         src/cvars.hpp
         src/engine.hpp
)

set(DATA_PATH "${opendf_SOURCE_DIR}")
configure_file("${opendf_SOURCE_DIR}/settings.cfg.in"
               "${opendf_BINARY_DIR}/settings.cfg")

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
