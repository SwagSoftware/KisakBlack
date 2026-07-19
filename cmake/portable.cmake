if (NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "The portable target currently supports Linux hosts")
endif()

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

file(GLOB_RECURSE KISAK_PORTABLE_CPP CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/DemonWare/*.cpp"
    "${SRC_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tl/*.cpp"
)
list(FILTER KISAK_PORTABLE_CPP EXCLUDE REGEX "/src/(libs|tracy|steam|nvapi|binklib|win32)/")
list(FILTER KISAK_PORTABLE_CPP EXCLUDE REGEX "/(gfx_gl|audio_openal)/tests/")

file(GLOB KISAK_ZLIB_C CONFIGURE_DEPENDS "${SRC_DIR}/zlib/*.c")
file(GLOB KISAK_JPEG_C CONFIGURE_DEPENDS "${SRC_DIR}/jpeg/*.c")
list(FILTER KISAK_JPEG_C EXCLUDE REGEX "/jmem(ansi|name)\\.c$")

add_executable(${BIN_NAME}
    ${KISAK_PORTABLE_CPP}
    ${KISAK_ZLIB_C}
    ${KISAK_JPEG_C}
)

target_include_directories(${BIN_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${SRC_DIR}"
    "${SRC_DIR}/platform/winsdk"
    "${SRC_DIR}/libs"
    "${SRC_DIR}/libs/libtomcrypt-1.17/src/headers"
    "${SRC_DIR}/libs/libtommath-1.0"
    "${SRC_DIR}/libs/libvpx-1.5.0/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/DemonWare"
    "${CMAKE_CURRENT_SOURCE_DIR}/tl"
    "${SRC_DIR}/jpeg"
)

target_compile_definitions(${BIN_NAME} PRIVATE KISAK_MP)
target_compile_options(${BIN_NAME} PRIVATE
    -m32
    -fpermissive
    $<$<COMPILE_LANGUAGE:CXX>:-malign-double;-mmmx;-msse;-msse2;-fms-extensions;-Wno-narrowing;-include;${SRC_DIR}/platform/compat/msvc_compat.h>
    $<$<COMPILE_LANGUAGE:C>:-D__cdecl=;-D__stdcall=;-D__fastcall=;-D__int8=char;-D__int16=short;-D__int32=int>
    -w
)
target_link_options(${BIN_NAME} PRIVATE -m32 -rdynamic)
target_link_directories(${BIN_NAME} PRIVATE /usr/lib/i386-linux-gnu)
target_link_libraries(${BIN_NAME} PRIVATE
    SDL2
    GLEW
    GL
    openal
    speex
    vpx
    pthread
    m
    dl
)

set_target_properties(${BIN_NAME} PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS ON
    OUTPUT_NAME blackops
)
