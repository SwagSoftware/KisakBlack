#!/bin/bash
# build_linux.sh — compile + link the whole engine into a Linux i386 executable.
# Requires (32-bit): g++-multilib, libSDL2-dev:i386, libglew-dev:i386, libopenal-dev:i386,
# libspeex-dev:i386, libvpx-dev:i386, libjpeg62-turbo:i386, zlib1g:i386.
set -e
cd "$(dirname "$0")"
mkdir -p build_linux/obj
INCS="-I. -Isrc -Isrc/platform/winsdk -Isrc/libs -Isrc/libs/libtomcrypt-1.17/src/headers -Isrc/libs/libtommath-1.0 -Isrc/libs/libvpx-1.5.0/include -IDemonWare -Itl -Isrc/jpeg"
CXXFLAGS="-m32 -malign-double -mmmx -msse -msse2 -std=c++20 -c -g -fms-extensions -w -fpermissive -Wno-narrowing -include src/platform/compat/msvc_compat.h -DKISAK_MP"
CFLAGS="-m32 -std=gnu11 -w -fpermissive -c -D__cdecl= -D__stdcall= -D__fastcall= -D__int8=char -D__int16=short -D__int32=int"

# engine C++ TUs (everything except win32/, vendored libs, tracy, tests, audio backends)
# NOTE: git's '**' pathspec only matches with an intermediate directory, so
# 'tl/**/*.cpp' silently drops top-level files like tl/tl_system.cpp. A plain
# '<dir>/*.cpp' pathspec matches recursively, so use that form for every tree.
mapfile -t CPP < <(git ls-files 'src/*.cpp' 'DemonWare/*.cpp' 'tl/*.cpp' 'src/platform/linux/*.cpp' 'src/audio_openal/al_audio.cpp' \
  | grep -vE '^src/(libs|tracy|steam|nvapi|binklib|win32)/' | grep -vE 'gfx_gl/tests/|audio_openal/tests/')
echo "compiling ${#CPP[@]} C++ TUs..."
printf '%s\n' "${CPP[@]}" | xargs -P"$(nproc)" -I{} bash -c 'o="build_linux/obj/$(echo "{}"|tr / _).o"; g++ '"$CXXFLAGS"' '"$INCS"' "{}" -o "$o"'

# bundled zlib C (the engine expects 1.2.3; system libz is newer)
for f in $(git ls-files 'src/zlib/*.c'); do
  gcc $CFLAGS -Isrc/zlib "$f" -o "build_linux/obj/$(echo "$f"|tr / _).o"
done

# bundled jpeg C (keep only the jmemnobs memory manager)
for f in $(git ls-files 'src/jpeg/*.c' | grep -vE 'jmemansi|jmemname'); do
  gcc $CFLAGS -I. -Isrc -Isrc/jpeg -Isrc/platform/winsdk "$f" -o "build_linux/obj/$(echo "$f"|tr / _).o"
done

echo "linking..."
g++ -m32 -rdynamic build_linux/obj/*.o -o build_linux/blackops -L/usr/lib/i386-linux-gnu \
    -lSDL2 -lGLEW -lGL -lopenal -lspeex -lvpx -l:libjpeg.so.62 -lpthread -lm -ldl
echo "built build_linux/blackops"
