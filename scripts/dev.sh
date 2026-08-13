#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "=========================================="
echo "         XyrisOS Development"
echo "=========================================="

echo ""
echo "[1/2] Building..."
"$PROJECT_ROOT/scripts/build.sh"

echo ""
echo "[2/2] Creating ISO and running..."
"$PROJECT_ROOT/scripts/run.sh"
