cmake_minimum_required(VERSION 3.9)
project(lab7 C)

set(CMAKE_C_STANDARD 99)
add_executable(klienci zad1/klienci.c zad1/shared.c)
add_executable(golibroda zad1/golibroda.c zad1/shared.c)

