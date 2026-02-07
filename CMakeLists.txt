cmake_minimum_required(VERSION 3.15.0)
project(pconvert)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib)

find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PythonLibs)

add_definitions(-DNO_PRAGMA_LIB)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

aux_source_directory(src/pconvert SOURCES)
add_executable(pconvert ${SOURCES})

target_include_directories(pconvert PUBLIC ${PYTHON_INCLUDE_PATH})
target_link_libraries(pconvert PNG::PNG ZLIB::ZLIB ${PYTHON_LIBRARY})
