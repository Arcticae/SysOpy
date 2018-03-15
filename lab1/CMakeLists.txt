cmake_minimum_required(VERSION 3.9)

project(lab1 LANGUAGES C) #defining language


set(CMAKE_POSITION_INDEPENDENT_CODE ON) #instead of fpic - on all targets
add_compile_options(-Wall)  #to ignore compile warnings
add_library(Blocks STATIC library.c library.h) #STATIC VERSION - CAN BE SHARED OR MODULE

add_executable(Main main.c) #adding the main exec
set_target_properties(Main PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED ON) #setting c standard
target_link_libraries(Main Blocks) #linking the libraries
