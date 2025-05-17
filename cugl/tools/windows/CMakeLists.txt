# A CMake shiv for Windows external library support
# Without this file, Windows will drop all library information on the floor
cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Main VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

__EXTERNALS__

# add the executable
add_executable(Main main.cpp)

target_link_libraries(Main PUBLIC ${EXTRA_LIBS})
