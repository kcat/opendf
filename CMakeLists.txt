cmake_minimum_required(VERSION 2.6)

project(opendf)

include(CheckCXXCompilerFlag)

check_cxx_compiler_flag(-std=c++11 HAVE_STD_CXX11)
if(HAVE_STD_CXX11)
    set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
endif()

add_executable(opendf main.cpp)

install(TARGETS opendf RUNTIME DESTINATION bin)
