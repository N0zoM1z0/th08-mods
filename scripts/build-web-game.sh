#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"
build_dir="${repo_root}/build/web-game"
dist_dir="${repo_root}/build/web-dist"
image="emscripten/emsdk:6.0.8@sha256:f174124ff798a3ead1abef247d9a849c270b642d552fea500a42565ff210f765"

cmake -E make_directory "${build_dir}" "${repo_root}/build/emscripten-cache"

docker run --rm \
    --volume "${repo_root}:/src" \
    --workdir /src \
    --user "$(id -u):$(id -g)" \
    "${image}" \
    env EM_CACHE=/src/build/emscripten-cache \
    emcmake cmake -S /src -B /src/build/web-game -G "Unix Makefiles" \
        -DCMAKE_BUILD_TYPE=Debug

docker run --rm \
    --volume "${repo_root}:/src" \
    --workdir /src \
    --user "$(id -u):$(id -g)" \
    "${image}" \
    env EM_CACHE=/src/build/emscripten-cache \
    cmake --build /src/build/web-game --target th08-web --parallel 1

echo "Built ${build_dir}/th08-web.html"

cmake -E make_directory "${dist_dir}"
for artifact in th08-web.html th08-web.js th08-web.wasm th08-web-icon.png; do
    cmake -E copy_if_different "${build_dir}/${artifact}" "${dist_dir}/${artifact}"
done

python3 "${repo_root}/scripts/check-web-provenance.py" --artifact "${dist_dir}"
echo "Staged ${dist_dir}"
