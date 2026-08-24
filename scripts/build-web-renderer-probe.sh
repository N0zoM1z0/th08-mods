#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"
output_dir="${repo_root}/build/web-renderer-probe"
image="emscripten/emsdk:6.0.8@sha256:f174124ff798a3ead1abef247d9a849c270b642d552fea500a42565ff210f765"

cmake -E make_directory "${output_dir}" "${repo_root}/build/emscripten-cache"

docker run --rm \
    --volume "${repo_root}:/src" \
    --workdir /src \
    --user "$(id -u):$(id -g)" \
    --env EM_CACHE=/src/build/emscripten-cache \
    "${image}" \
    em++ \
        experiments/web-renderer/renderer_probe.cpp \
        src/modern/linux/d3d8_compat.cpp \
        -std=c++98 \
        -O1 \
        -fms-extensions \
        -Wno-address-of-temporary \
        -Wno-ignored-pragmas \
        -Wno-write-strings \
        -DTH08_MODERN_PORT \
        -DTH08_MODERN_LINUX \
        -DTH08_MODERN_WEB \
        -DWIN32_LEAN_AND_MEAN \
        -Isrc/modern/linux/include \
        -Isrc \
        -include src/modern/web/web_compat.hpp \
        --use-port=sdl2 \
        -sLEGACY_GL_EMULATION \
        -sGL_FFP_ONLY \
        -sMIN_WEBGL_VERSION=2 \
        -sMAX_WEBGL_VERSION=2 \
        -sEXIT_RUNTIME=0 \
        -o build/web-renderer-probe/index.html
