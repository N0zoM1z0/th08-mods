#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"
source_dir="${repo_root}/experiments/web-data-mount"
output_dir="${repo_root}/build/web-data-mount"
image="emscripten/emsdk:6.0.8@sha256:f174124ff798a3ead1abef247d9a849c270b642d552fea500a42565ff210f765"

cmake -E make_directory "${output_dir}" "${repo_root}/build/emscripten-cache"
cmake -E copy_if_different "${source_dir}/index.html" "${output_dir}/index.html"
cmake -E copy_if_different "${source_dir}/worker.js" "${output_dir}/worker.js"

docker run --rm \
    --volume "${repo_root}:/src" \
    --workdir /src \
    --user "$(id -u):$(id -g)" \
    --env EM_CACHE=/src/build/emscripten-cache \
    "${image}" \
    em++ experiments/web-data-mount/probe.cpp \
        --no-entry \
        -O1 \
        -lworkerfs.js \
        -sMODULARIZE=1 \
        -sEXPORT_NAME=createDataProbe \
        -sENVIRONMENT=worker \
        -sEXPORTED_FUNCTIONS=_inspect_data_files \
        -sEXPORTED_RUNTIME_METHODS=FS,WORKERFS \
        -o build/web-data-mount/probe.js
