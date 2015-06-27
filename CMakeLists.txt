cmake_minimum_required(VERSION 2.6)
project(opendf)

add_executable(opendf main.cpp)

install(TARGETS opendf RUNTIME DESTINATION bin)
