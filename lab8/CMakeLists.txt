cmake_minimum_required(VERSION 3.9)
project(lab8 C)

set(CMAKE_C_STANDARD 99)

add_executable(main main.c)
add_executable(filter_generator filter_generator.c)
target_link_libraries(main pthread)
target_link_libraries(main m)