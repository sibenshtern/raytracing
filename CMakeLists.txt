cmake_minimum_required(VERSION 3.16)
project(mpi_raytracer C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT DEFINED MPI_HOME AND EXISTS "${CMAKE_SOURCE_DIR}/lib/openmpi-install/bin/mpicc")
    set(MPI_HOME "${CMAKE_SOURCE_DIR}/lib/openmpi-install" CACHE PATH "")
endif()
if(MPI_HOME)
    set(MPI_C_COMPILER "${MPI_HOME}/bin/mpicc" CACHE FILEPATH "")
endif()
find_package(MPI REQUIRED)

include(FetchContent)

set(FETCHCONTENT_QUIET OFF)

# --- zlib (dependency of libpng) ---
set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG        v1.3.1)
FetchContent_MakeAvailable(zlib)

set(ZLIB_INCLUDE_DIR "${zlib_SOURCE_DIR}" CACHE PATH   "" FORCE)
set(ZLIB_LIBRARY     zlibstatic           CACHE STRING "" FORCE)
set(ZLIB_LIBRARIES   zlibstatic           CACHE STRING "" FORCE)

if(NOT TARGET ZLIB::ZLIB)
    add_library(ZLIB::ZLIB ALIAS zlibstatic)
endif()

# --- libpng (PNG writer) ---
set(PNG_SHARED       OFF CACHE BOOL "" FORCE)
set(PNG_STATIC       ON  CACHE BOOL "" FORCE)
set(PNG_TESTS        OFF CACHE BOOL "" FORCE)
set(PNG_TOOLS        OFF CACHE BOOL "" FORCE)
set(PNG_EXECUTABLES  OFF CACHE BOOL "" FORCE)
set(SKIP_INSTALL_ALL ON  CACHE BOOL "" FORCE)
FetchContent_Declare(png
    GIT_REPOSITORY https://github.com/pnggroup/libpng.git
    GIT_TAG        v1.6.43)
FetchContent_MakeAvailable(png)

add_executable(raytracer
    src/main.c
    src/scene.c
    src/render.c
    src/image_io.c)

target_include_directories(raytracer PRIVATE
    src
    ${png_SOURCE_DIR}
    ${png_BINARY_DIR}
    ${zlib_SOURCE_DIR}
    ${zlib_BINARY_DIR})

target_link_libraries(raytracer PRIVATE
    MPI::MPI_C
    png_static
    zlibstatic)

target_compile_options(raytracer PRIVATE -O3 -Wall -Wextra -Wno-unused-parameter -march=native -ffast-math -funroll-loops)

find_library(M_LIB m)
if(M_LIB)
    target_link_libraries(raytracer PRIVATE ${M_LIB})
endif()
