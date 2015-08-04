cmake_minimum_required(VERSION 2.6)

project(opendf)

set(CMAKE_MODULE_PATH "${opendf_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")

include(CheckCXXCompilerFlag)

check_cxx_compiler_flag(-std=c++11 HAVE_STD_CXX11)
if(HAVE_STD_CXX11)
    set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
endif()

if(MSVC)
    add_definitions("-DNOMINMAX")
    set(CMAKE_EXE_LINKER_FLAGS "/FORCE:Multiple ${CMAKE_EXE_LINKER_FLAGS}")
else()
    check_cxx_compiler_flag("-Wall" HAVE_WALL_SWITCH)
    if(HAVE_WALL_SWITCH)
        add_definitions("-Wall")
    endif()
    check_cxx_compiler_flag("-Wextra" HAVE_WEXTRA_SWITCH)
    if(HAVE_WEXTRA_SWITCH)
        add_definitions("-Wextra")
    endif()
    # MyGUI triggers a lot of unused parameter warnings...
    check_cxx_compiler_flag("-Wno-unused-parameter" HAVE_WNO_UNUSED_PARAMETER_SWITCH)
    if(HAVE_WNO_UNUSED_PARAMETER_SWITCH)
        add_definitions("-Wno-unused-parameter")
    endif()
endif()


find_package(OpenSceneGraph REQUIRED osgDB osgViewer osgGA osgUtil)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(MyGUI REQUIRED)

include_directories("${opendf_SOURCE_DIR}/src")

set(SRCS src/components/sdlutil/graphicswindow.cpp
         src/components/settings/configfile.cpp
         src/components/archives/archive.cpp
         src/components/archives/bsaarchive.cpp
         src/components/vfs/manager.cpp
         src/components/vfs/osg_callbacks.cpp
         src/components/resource/texturemanager.cpp
         src/components/resource/meshmanager.cpp
         src/components/mygui_osg/rendermanager.cpp
         src/components/mygui_osg/texture.cpp
         src/components/mygui_osg/vertexbuffer.cpp
         src/components/mygui_osg/datamanager.cpp
         src/components/dfosg/texloader.cpp
         src/components/dfosg/meshloader.cpp
         src/opendf/input/input.cpp
         src/opendf/render/pipeline.cpp
         src/opendf/render/renderer.cpp
         src/opendf/gui/gui.cpp
         src/opendf/class/placeable.cpp
         src/opendf/class/activator.cpp
         src/opendf/class/linker.cpp
         src/opendf/class/mover.cpp
         src/opendf/class/door.cpp
         src/opendf/world/world.cpp
         src/opendf/world/pitems.cpp
         src/opendf/world/ditems.cpp
         src/opendf/world/mblocks.cpp
         src/opendf/world/dblocks.cpp
         src/opendf/log.cpp
         src/opendf/cvars.cpp
         src/opendf/engine.cpp
         src/opendf/main.cpp
)

set(HDRS src/misc/sparsearray.hpp
         src/components/sdlutil/graphicswindow.hpp
         src/components/settings/configfile.hpp
         src/components/archives/archive.hpp
         src/components/archives/bsaarchive.hpp
         src/components/vfs/manager.hpp
         src/components/vfs/osg_callbacks.hpp
         src/components/resource/texturemanager.hpp
         src/components/resource/meshmanager.hpp
         src/components/mygui_osg/diagnostic.h
         src/components/mygui_osg/rendermanager.h
         src/components/mygui_osg/texture.h
         src/components/mygui_osg/vertexbuffer.h
         src/components/mygui_osg/datamanager.h
         src/components/dfosg/texloader.hpp
         src/components/dfosg/meshloader.hpp
         src/opendf/input/input.hpp
         src/opendf/render/pipeline.hpp
         src/opendf/render/renderer.hpp
         src/opendf/gui/iface.hpp
         src/opendf/gui/gui.hpp
         src/opendf/class/placeable.hpp
         src/opendf/class/activator.hpp
         src/opendf/class/linker.hpp
         src/opendf/class/mover.hpp
         src/opendf/class/door.hpp
         src/opendf/world/iface.hpp
         src/opendf/world/world.hpp
         src/opendf/world/itembase.hpp
         src/opendf/world/pitems.hpp
         src/opendf/world/ditems.hpp
         src/opendf/world/mblocks.hpp
         src/opendf/world/dblocks.hpp
         src/opendf/log.hpp
         src/opendf/cvars.hpp
         src/opendf/engine.hpp
)

if(WIN32)
    set(SRCS src/misc/fnmatch.c
             ${SRCS}
    )

    set(HDRS src/misc/fnmatch.h
             ${HDRS}
    )
endif()

set(DATA_PATH "${opendf_SOURCE_DIR}")
configure_file("${opendf_SOURCE_DIR}/settings.cfg.in"
               "${opendf_BINARY_DIR}/settings.cfg")

add_executable(opendf ${SRCS} ${HDRS})
set_property(TARGET opendf APPEND PROPERTY INCLUDE_DIRECTORIES
    "${opendf_SOURCE_DIR}/src/opendf"
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


set(SRCS src/components/archives/archive.cpp
         src/components/archives/bsaarchive.cpp
         src/bsatool/bsatool.cpp
)
set(HDRS src/components/archives/archive.hpp
         src/components/archives/bsaarchive.hpp
)

add_executable(bsatool ${SRCS} ${HDRS})


install(TARGETS opendf bsatool RUNTIME DESTINATION bin)
