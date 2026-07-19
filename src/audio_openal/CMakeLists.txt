# Standalone, non-Windows build of the OpenAL audio backend + a smoke test. This is
# the audio analogue of src/gfx_gl/CMakeLists.txt: it lets the XAudio2→OpenAL layer be
# built and run-tested in isolation while it is brought up.
#   cmake -S src/audio_openal -B build_al && cmake --build build_al && ./build_al/al_smoke
cmake_minimum_required(VERSION 3.16)
project(kisak_al CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED WINSDK_INC)
    set(WINSDK_INC "${CMAKE_CURRENT_SOURCE_DIR}/../platform/winsdk")
endif()

find_library(OPENAL_LIB openal REQUIRED)

add_library(kisak_al STATIC al_audio.cpp)
target_include_directories(kisak_al PUBLIC
    "${WINSDK_INC}" "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(kisak_al PRIVATE
    -fms-extensions -fpermissive -w
    -include "${CMAKE_CURRENT_SOURCE_DIR}/../platform/compat/msvc_compat.h" -DKISAK_MP)
target_link_libraries(kisak_al PUBLIC ${OPENAL_LIB})

add_executable(al_smoke tests/al_smoke.cpp)
target_link_libraries(al_smoke PRIVATE kisak_al)
target_compile_options(al_smoke PRIVATE
    -fms-extensions -fpermissive -w
    -include "${CMAKE_CURRENT_SOURCE_DIR}/../platform/compat/msvc_compat.h" -DKISAK_MP)
