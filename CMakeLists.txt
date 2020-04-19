cmake_minimum_required(VERSION 3.0.0)
project(pconvert)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package(PythonLibs)

add_definitions(-DNO_PRAGMA_LIB)

add_executable(pconvert src/pconvert/stdafx.c src/pconvert/pconvert.c src/pconvert/structs.c src/pconvert/util.c src/pconvert/opencl.c)

target_include_directories(pconvert PUBLIC ${PYTHON_INCLUDE_PATH})
target_link_libraries(pconvert ${CONAN_LIBS} ${PYTHON_LIBRARY})
