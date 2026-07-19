cmake_minimum_required(VERSION 3.16)
project(KisakBlack)

set(BIN_NAME "KisakBlack")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
if (NOT CMAKE_GENERATOR_PLATFORM)
    set(CMAKE_GENERATOR_PLATFORM "Win32")
endif()

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(TL_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/tl")
set(DW_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/DemonWare")

include(cmake_files.cmake)

# phys_gjk.cpp specific flag (experimental)
set_source_files_properties(
    "${SRC_DIR}/physics/phys_gjk.cpp"
    PROPERTIES
    COMPILE_OPTIONS "/arch:IA32"
)

# ----- Preprocessor Defines -----
# Common to both Debug and Release
target_compile_definitions(KisakBlack PUBLIC
    WIN32
    WIN32_LEAN_AND_MEAN
    NOMINMAX
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
target_include_directories(KisakBlack PUBLIC "C:\\Program Files (x86)\\Microsoft DirectX SDK (June 2010)\\Include")

# ----- Library directories -----
target_link_directories(KisakBlack PUBLIC "C:\\Program Files (x86)\\Microsoft DirectX SDK (June 2010)\\Lib\\x86")
target_link_directories(KisakBlack PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/libs/libvpx-1.5.0")
target_link_directories(KisakBlack PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/steam/redistributable_bin")
target_link_directories(KisakBlack PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/binklib")
target_link_directories(KisakBlack PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/nvapi")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT  /O2 /MP /W3 ${MSVC_WARNING_DISABLES} /permissive")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} /MTd /Od /MP /W3 ${MSVC_WARNING_DISABLES} /permissive")

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
else()
    include(cmake/portable.cmake)
endif()
