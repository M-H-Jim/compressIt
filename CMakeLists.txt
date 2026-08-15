cmake_minimum_required(VERSION 4.0)

project(compressIt C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(compressIt
    src/main.c
    src/huffman.c
)

target_include_directories(compressIt PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party/include
)

target_link_directories(compressIt PRIVATE
    ${CMAKE_SOURCE_DIR}/lib
)

target_link_libraries(compressIt PRIVATE
    glfw3dll
    glew32
    opengl32
    gdi32
)
