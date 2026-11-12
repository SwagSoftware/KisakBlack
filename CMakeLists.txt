cmake_minimum_required(VERSION 3.16)
project(KisakBlack)

set(BIN_NAME "KisakBlack")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_GENERATOR_PLATFORM "WIN32")

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(TL_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/tl")
set(DW_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/DemonWare")

include(cmake_files.cmake)

# DirectX SDK (June 2010): -DDXSDK_DIR=<folder with Include\ and Lib\>, or the DXSDK_DIR environment variable
# (the installer sets it), or the default install folder. An extracted copy of the SDK works too.
if (NOT DXSDK_DIR)
    if (DEFINED ENV{DXSDK_DIR})
        set(DXSDK_DIR "$ENV{DXSDK_DIR}")
    else()
        set(DXSDK_DIR "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)")
    endif()
endif()
if (NOT EXISTS "${DXSDK_DIR}/Include/d3dx9.h")
    message(FATAL_ERROR "DirectX SDK (June 2010) not found at '${DXSDK_DIR}'. Install it or pass -DDXSDK_DIR=<folder>.")
endif()

# ----- Preprocessor Defines -----
# Common to both Debug and Release
target_compile_definitions(KisakBlack PUBLIC
    WIN32
    _CONSOLE
    KISAK_MP
    _CRT_SECURE_NO_WARNINGS
    _WINSOCK_NO_DEPRECATED_NO_WARNINGS
)
# Debug-only
target_compile_definitions(KisakBlack PUBLIC
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Debug>:TRACY_ENABLE>
    $<$<CONFIG:Debug>:TRACY_ON_DEMAND>
)
# Release-only
target_compile_definitions(KisakBlack PUBLIC
    $<$<CONFIG:Release>:NDEBUG>
)

# ----- Include directories -----
target_include_directories(KisakBlack PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(KisakBlack PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories(KisakBlack PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/libs")
target_include_directories(KisakBlack PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/libs/libtomcrypt-1.17/src/headers")
target_include_directories(KisakBlack PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/libs/libtommath-1.0")
target_include_directories(KisakBlack PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/libs/libvpx-1.5.0/include")
target_include_directories(KisakBlack PUBLIC "${DXSDK_DIR}/Include")

# ----- Library directories -----
target_link_directories(KisakBlack PUBLIC "${DXSDK_DIR}/Lib/x86")
target_link_directories(KisakBlack PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/libs/libvpx-1.5.0")
target_link_directories(KisakBlack PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/steam/redistributable_bin")
target_link_directories(KisakBlack PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/binklib")
target_link_directories(KisakBlack PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/nvapi")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT  /O2 /MP /W3 ${MSVC_WARNING_DISABLES} /permissive")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} /MTd /Od /MP /W3 ${MSVC_WARNING_DISABLES} /permissive")

# ----- Physics: TreyBO (git submodule; `git submodule update --init`) -----
# Added after the flags above so the library is built with the same CRT (/MT, /MTd).
if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/TreyBO/CMakeLists.txt")
    message(FATAL_ERROR "TreyBO submodule is missing. Run: git submodule update --init")
endif()
set(TREYBO_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(TREYBO_BUILD_DEMO  OFF CACHE BOOL "" FORCE)
add_subdirectory(TreyBO)
target_link_libraries(KisakBlack PUBLIC treybo)

if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
    target_link_options(KisakBlack PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
    target_compile_options(KisakBlack PRIVATE $<$<CONFIG:Debug>:/ZI>)

    set(CMAKE_CXX_FLAGS_DEBUG           "${CMAKE_CXX_FLAGS_DEBUG} /SAFESEH:NO")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /SAFESEH:NO")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /SAFESEH:NO")
    set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} /SAFESEH:NO")
endif()

# ----- Links -----
target_link_libraries(KisakBlack PUBLIC
    steam_api.lib
    binkw32.lib
    nvapi.lib
    d3d9.lib
    ws2_32.lib
    dxguid.lib
    dsound.lib
    winmm.lib
    xinput.lib
    xapobase.lib
    dxerr.lib
    libvpx.lib
    # June-2010 DirectX SDK libs (xapobase, dxerr) reference pre-VS2015 CRT
    # symbols (_vsnprintf, etc.) that the modern UCRT no longer exports.
    # legacy_stdio_definitions.lib provides shims so those static libs link.
    legacy_stdio_definitions.lib
)
# ----- Debug Links -----
target_link_libraries(KisakBlack PUBLIC $<$<CONFIG:Debug>:d3dx9d.lib>)
# ----- Release Links -----
target_link_libraries(KisakBlack PUBLIC $<$<CONFIG:Release>:d3dx9.lib>)
target_link_options(KisakBlack PRIVATE /machine:x86)

# ----- Linker Options -----
# /DYNAMICBASE:NO - Turning this off causes directx to crash "directx has encountered an unrecoverable error". 
# /LARGEADDRESSAWARE - for convenience (4gb)
# /DEBUG - PDB for both configs 
target_link_options(KisakBlack PRIVATE /DYNAMICBASE:NO /LARGEADDRESSAWARE /DEBUG)

set_target_properties(KisakBlack PROPERTIES WIN32_EXECUTABLE TRUE)

# Set the VS debugger's working directory to the build-output dir for whichever config is active 
set_target_properties(KisakBlack PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)
