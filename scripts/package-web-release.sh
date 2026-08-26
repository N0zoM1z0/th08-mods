#!/usr/bin/env bash
set -euo pipefail

usage() {
    echo "usage: $0 <version> [output.tar.gz]"
}

if [[ $# -eq 1 && ( "$1" == "-h" || "$1" == "--help" ) ]]; then
    usage
    exit 0
fi
if [[ $# -lt 1 || $# -gt 2 ]]; then
    usage >&2
    exit 2
fi

version="$1"
if [[ ! "${version}" =~ ^v[0-9]+\.[0-9]+\.[0-9]+([.-][0-9A-Za-z.-]+)?$ ]]; then
    echo "error: version must look like v1.2.3" >&2
    exit 2
fi

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"
artifact_dir="${repo_root}/build/web-dist"
output="${2:-${repo_root}/build/releases/th08-web-${version}.tar.gz}"

if [[ "${output}" != /* ]]; then
    output="${repo_root}/${output}"
fi

artifacts=(
    _headers
    _redirects
    th08-web.html
    th08-web.js
    th08-web.wasm
    th08-web-firefox.html
    th08-web-firefox.js
    th08-web-firefox.wasm
    th08-web-icon.png
)

python3 "${script_dir}/check-web-provenance.py" --artifact "${artifact_dir}"

mkdir -p "$(dirname "${output}")"
tar \
    --sort=name \
    --mtime='UTC 1970-01-01' \
    --owner=0 \
    --group=0 \
    --numeric-owner \
    --mode=0644 \
    --format=ustar \
    -C "${artifact_dir}" \
    -cf - \
    "${artifacts[@]}" \
    | gzip -n > "${output}"

output_dir="$(dirname "${output}")"
output_name="$(basename "${output}")"
(
    cd "${output_dir}"
    sha256sum "${output_name}" > "${output_name}.sha256"
)

echo "Packaged ${output}"
echo "Wrote ${output}.sha256"
