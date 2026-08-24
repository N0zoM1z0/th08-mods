#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"
image="emscripten/emsdk:6.0.8@sha256:f174124ff798a3ead1abef247d9a849c270b642d552fea500a42565ff210f765"

docker run --rm \
    --volume "${repo_root}:/src" \
    --workdir /src \
    --user "$(id -u):$(id -g)" \
    "${image}" \
    emcmake cmake -S /src -B /src/build/web-probe -G "Unix Makefiles" \
        -DCMAKE_BUILD_TYPE=Debug

docker run --rm \
    --volume "${repo_root}:/src" \
    --workdir /src \
    --user "$(id -u):$(id -g)" \
    "${image}" \
    cmake --build /src/build/web-probe --parallel 1
