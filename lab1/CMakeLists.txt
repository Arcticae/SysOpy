
cmake_minimum_required(VERSION 3.9)
project(lab1 LANGUAGES C) #defining language

set(CMAKE_C_STANDARD 99)
add_compile_options(-Wall)  #to ignore compile warnings

option(SHARED "Compilation with dynamic library" OFF)
option(STATIC "Static compilation of library" OFF)
option(MODULE "Loading functions on-the-fly" OFF)
option(OPTIMIZATION "Level of optimizing by compiler" -o)

set(LIBRARY library.h library.c)

macro(shared)
    add_library(shared_lib SHARED ${LIBRARY})
    add_executable(Main main.c)
    target_link_libraries(Main shared_lib)
    target_compile_options(shared_lib PUBLIC -o${OPTIMIZATION})
    target_compile_options(Main PUBLIC -o${OPTIMIZATION})
endmacro(shared)

macro(static)
    add_library(static_lib STATIC ${LIBRARY})
    add_executable(Main main.c)
    target_link_libraries(Main static_lib)
    target_compile_options(static_lib PUBLIC -o${OPTIMIZATION})
    target_compile_options(Main PUBLIC -o${OPTIMIZATION})
endmacro(static)

macro(module)
    add_library(module_lib MODULE ${LIBRARY})
    set(CMAKE_POSITION_INDEPENDENT_CODE ON) #instead of fpic - on all targets
    add_executable(Main main_dynamic.c)
    target_compile_options(module_lib PUBLIC -o${OPTIMIZATION})
    target_link_libraries(Main dl)
    target_compile_options(Main PUBLIC -o${OPTIMIZATION})
endmacro(module)
set(OPTIPIZATION 2)
message(OPTIMIZATION)
static()

enable_testing()
add_test(t1 Main 0 2000 2000 add 2000 remove 2000)
add_test(t2 Main 1 3000 3000 add_and_remove 2000 search_element 2000)
add_test(t3 Main 0 2500 4000 add 2000 remove 2000)
add_test(t4 Main 1 2600 5000 add 2000 remove 2000)
add_test(t5 Main 0 2700 6000 search_element 2000 add_and_remove 2000)
add_test(t6 Main 1 2900 7000 add 2000 remove 2000)
add_test(t7 Main 0 2800 8000 remove 2000 remove 2000)
add_test(t8 Main 1 2600 9000 add 2000 add 2000)
add_test(t9 Main 0 4000 10000 search_element 2000 remove 2000)
