cmake_minimum_required(VERSION 3.10)

project(calc-apps C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Do not specify /O2 because there are Debug/Release modes.
if (MSVC)
    add_compile_options(/W4 /WX /D_CRT_SECURE_NO_WARNINGS)  # Disable warnings about VS-only _s functions (e.g. scanf_s).
else()
    if (NOT CMAKE_BUILD_TYPE)
      set(CMAKE_BUILD_TYPE Debug CACHE STRING "Default build type from CMakeLists.txt" FORCE)  # Add debug information
    endif (NOT CMAKE_BUILD_TYPE)

    add_compile_options(-Wall -Wextra -Werror)
    if (UNIX AND NOT CMAKE_CXX_FLAGS)  # Do not add if -DCMAKE_CXX_FLAGS is passed
      add_compile_options(-fsanitize=address -fsanitize=undefined)
      add_link_options(-fsanitize=address -fsanitize=undefined)
    endif (UNIX AND NOT CMAKE_CXX_FLAGS)
endif (MSVC)

include(CheckLibraryExists)
CHECK_LIBRARY_EXISTS(m sqrt "" HAVE_LIB_M)
if (HAVE_LIB_M)
    link_libraries(m)
endif (HAVE_LIB_M)

add_subdirectory(lib)  # Add only after all compilation options are specified.

add_executable(calc-test doctest_main.cpp calc_test.cpp)
target_link_libraries(calc-test calc)

add_executable(calc-test-c calc_test_c.c)
target_link_libraries(calc-test-c calc)

add_executable(calc-cli calc_cli.c)
target_link_libraries(calc-cli calc)

add_executable(demo-parser demo-parser.c)