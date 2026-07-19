# src/gfx_gl — OpenGL backend for the D3D9 translation layer.
#
# Standalone, non-Windows build of the GL backend + its tests. This is NOT wired
# into the main Windows game build (which uses the real DirectX SDK); it lets the
# GL layer be built and run-tested in isolation on Linux/macOS while it is brought
# up. Usage:  cmake -S src/gfx_gl -B build_gl && cmake --build build_gl && ./build_gl/gl_smoke
cmake_minimum_required(VERSION 3.16)
project(kisak_gl CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(GLEW REQUIRED glew)
find_package(OpenGL REQUIRED)

# Portable D3D9 SDK headers (so <d3d9.h> resolves to ours on non-Windows).
set(WINSDK_INC "${CMAKE_CURRENT_SOURCE_DIR}/../platform/winsdk")

add_library(kisak_gl STATIC
    glcontext_sdl.cpp
    gl_d3d9.cpp
    gl_d3d9_draw.cpp
    gl_resources.cpp
    gl_state.cpp
    gl_shader.cpp
    gl_program.cpp
    gl_query.cpp
    gl_format.cpp
    gl_surface_ops.cpp
)
target_include_directories(kisak_gl PUBLIC
    "${WINSDK_INC}" "${CMAKE_CURRENT_SOURCE_DIR}"
    ${SDL2_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})
target_link_libraries(kisak_gl PUBLIC
    ${SDL2_LIBRARIES} ${GLEW_LIBRARIES} OpenGL::GL)
target_compile_options(kisak_gl PRIVATE -Wall -Wextra)

add_executable(gl_smoke tests/smoke_clear.cpp)
target_link_libraries(gl_smoke PRIVATE kisak_gl)

add_executable(gl_triangle tests/smoke_triangle.cpp)
target_link_libraries(gl_triangle PRIVATE kisak_gl)

add_executable(gl_texture tests/smoke_texture.cpp)
target_link_libraries(gl_texture PRIVATE kisak_gl)

add_executable(gl_shader tests/smoke_shader.cpp)
target_link_libraries(gl_shader PRIVATE kisak_gl)

add_executable(gl_rendertarget tests/smoke_rendertarget.cpp)
target_link_libraries(gl_rendertarget PRIVATE kisak_gl)

add_executable(gl_query tests/smoke_query.cpp)
target_link_libraries(gl_query PRIVATE kisak_gl)

add_executable(gl_shader3d tests/smoke_shader3d.cpp)
target_link_libraries(gl_shader3d PRIVATE kisak_gl)

add_executable(gl_depthrt tests/smoke_depthrt.cpp)
target_link_libraries(gl_depthrt PRIVATE kisak_gl)

add_executable(gl_readback tests/smoke_readback.cpp)
target_link_libraries(gl_readback PRIVATE kisak_gl)

add_executable(gl_stretchrect tests/smoke_stretchrect.cpp)
target_link_libraries(gl_stretchrect PRIVATE kisak_gl)
