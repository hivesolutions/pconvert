cmake_minimum_required(VERSION 2.8.12)
project (pconvert)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(pconvert src/pconvert/stdafx.c src/pconvert/pconvert.c src/pconvert/structs.c src/pconvert/util.c src/pconvert/opencl.c)
target_link_libraries(pconvert ${CONAN_LIBS})
