#!/bin/bash
# Build (and optionally run) the native macOS PC port.
#
# Usage:
#   ./build-pc.sh                 build only
#   ./build-pc.sh --run [args]    build, then run ./pc/sotn with the given args
#                                 e.g. ./build-pc.sh --run --stage nz1 --scale 3
#   ./build-pc.sh --assets        re-extract + rebuild game assets first
#                                 (needed after editing config/assets.us.yaml)
set -euo pipefail
cd "$(dirname "$0")"

RUN=0
ASSETS=0
RUN_ARGS=()
while [[ $# -gt 0 ]]; do
    case "$1" in
    --run) RUN=1; shift; RUN_ARGS=("$@"); break ;;
    --assets) ASSETS=1; shift ;;
    *) echo "unknown option: $1 (use --run or --assets)"; exit 1 ;;
    esac
done

if [[ $ASSETS -eq 1 ]]; then
    echo "== extracting + building assets =="
    ./sotn.sh extract-assets
    ./sotn.sh build-assets
fi

# Always reconfigure: CMakeLists edits (e.g. new stage files) are otherwise
# silently ignored and you link stale objects.
echo "== configuring =="
cmake -S . -B pc -DCMAKE_BUILD_TYPE=RelWithDebInfo -DSOTN_ASAN=OFF

echo "== building =="
cmake --build pc -j "$(sysctl -n hw.ncpu)"

echo "== built: pc/sotn =="

if [[ $RUN -eq 1 ]]; then
    # Must run from repo root: the game uses relative asset paths.
    exec ./pc/sotn "${RUN_ARGS[@]}"
fi
