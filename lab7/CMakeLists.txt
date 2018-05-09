cmake_minimum_required(VERSION 3.9)
project(lab7 C)
add_compile_options(-std=gnu11)

add_executable(klienci zad2/klienci.c zad2/shared.c)
add_executable(golibroda zad2/golibroda.c zad2/shared.c)

target_link_libraries(golibroda rt)
target_link_libraries(golibroda pthread)
target_link_libraries(klienci rt)
target_link_libraries(klienci pthread)