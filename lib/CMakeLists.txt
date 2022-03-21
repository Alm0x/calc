cmake_minimum_required(VERSION 3.10)

project(calc-lib C)

# All compilation options are specified in the root project.

add_library(calc src/calc.c)
target_include_directories(calc PUBLIC include)
