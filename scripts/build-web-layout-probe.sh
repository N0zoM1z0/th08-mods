#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"
output_dir="${repo_root}/build/web-layout-probe"
image="emscripten/emsdk:6.0.8@sha256:f174124ff798a3ead1abef247d9a849c270b642d552fea500a42565ff210f765"

cmake -E make_directory "${output_dir}" "${repo_root}/build/emscripten-cache"

docker run --rm \
    --volume "${repo_root}:/src" \
    --workdir /src \
    --user "$(id -u):$(id -g)" \
    --env EM_CACHE=/src/build/emscripten-cache \
    "${image}" \
    em++ experiments/web-layout/layout_probe.cpp \
        -O1 \
        -sENVIRONMENT=node \
        -sGLOBAL_BASE=33554432 \
        -sINITIAL_HEAP=33554432 \
        -o build/web-layout-probe/probe.js

node "${output_dir}/probe.js"
