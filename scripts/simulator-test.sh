#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$PROJECT_ROOT"

echo "=========================================="
echo "      XyrisOS Simulator Test Suite"
echo "=========================================="
echo ""

if [ ! -d "$PROJECT_ROOT/simulator" ]; then
    echo "Error: simulator directory not found."
    echo "Expected: $PROJECT_ROOT/simulator"
    exit 1
fi

echo "[1/3] Configuring simulator..."

cmake -S "$PROJECT_ROOT/simulator" \
      -B "$PROJECT_ROOT/simulator/build" \
      -G Ninja

echo ""
echo "[2/3] Building simulator..."

cmake --build "$PROJECT_ROOT/simulator/build"

echo ""
echo "[3/3] Running tests..."

ctest --test-dir "$PROJECT_ROOT/simulator/build" --output-on-failure

echo ""
echo "=========================================="
echo "   Simulator tests passed successfully!"
echo "=========================================="