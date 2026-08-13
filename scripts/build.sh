#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$PROJECT_ROOT"

echo "=========================================="
echo "        XyrisOS Build System"
echo "=========================================="
echo ""

echo "[1/2] Configuring XyrisOS..."

CMAKE_ARGS=(
    -S "$PROJECT_ROOT"
    -B "$PROJECT_ROOT/build"
    -G Ninja
)

# A dedicated cross-toolchain is optional in the current repository.
# Use it when present; otherwise use the host GCC/Clang compiler.
if [ -f "$PROJECT_ROOT/toolchain/x86_64-toolchain.cmake" ]; then
    echo "Using dedicated x86_64 toolchain."
    CMAKE_ARGS+=(
        -DCMAKE_TOOLCHAIN_FILE="$PROJECT_ROOT/toolchain/x86_64-toolchain.cmake"
    )
else
    echo "No dedicated toolchain found; using the host compiler."
fi

cmake "${CMAKE_ARGS[@]}"

echo ""
echo "[2/2] Building XyrisOS..."

cmake --build "$PROJECT_ROOT/build"

echo ""
echo "=========================================="
echo "   XyrisOS build completed successfully!"
echo "=========================================="