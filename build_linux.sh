#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"

: "${CMAKE_BUILD_TYPE:=RelWithDebInfo}"
: "${CMAKE_BUILD_PARALLEL_LEVEL:=4}"
: "${KISAK_STATIC_RUNTIME_DEPS:=OFF}"
cmake -S . -B build_linux \
    -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
    -DKISAK_STATIC_RUNTIME_DEPS="$KISAK_STATIC_RUNTIME_DEPS"
cmake --build build_linux --parallel "$CMAKE_BUILD_PARALLEL_LEVEL"
