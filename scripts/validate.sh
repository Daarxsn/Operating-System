#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
SIM_BUILD_DIR="$PROJECT_ROOT/simulator/build"

cd "$PROJECT_ROOT"

echo "=========================================="
echo "       XyrisOS Validation Pipeline"
echo "=========================================="
echo "[1/5] Clean kernel build"
rm -rf "$BUILD_DIR"
cmake -S "$PROJECT_ROOT" -B "$BUILD_DIR" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$PROJECT_ROOT/toolchain/x86_64-toolchain.cmake"
cmake --build "$BUILD_DIR"

echo "[2/5] Unresolved-symbol audit"
if nm -u "$BUILD_DIR/kernel.elf" | grep -q .; then
    echo "ERROR: unresolved symbols detected"
    nm -u "$BUILD_DIR/kernel.elf"
    exit 1
fi

echo "PASS: no unresolved kernel symbols"

echo "[3/5] Simulator build and tests"
rm -rf "$SIM_BUILD_DIR"
cmake -S "$PROJECT_ROOT/simulator" -B "$SIM_BUILD_DIR" -G Ninja
cmake --build "$SIM_BUILD_DIR"
ctest --test-dir "$SIM_BUILD_DIR" --output-on-failure

echo "[4/5] ISO prerequisites"
if command -v xorriso >/dev/null 2>&1; then
    "$PROJECT_ROOT/scripts/iso.sh"
    ISO_STATUS=PASS
else
    echo "BLOCKED: xorriso is not installed; ISO generation not executed."
    ISO_STATUS=BLOCKED
fi

echo "[5/5] QEMU runtime"
if command -v qemu-system-x86_64 >/dev/null 2>&1 && [[ -f "$PROJECT_ROOT/XyrisOS.iso" ]]; then
        QEMU_LOG="$PROJECT_ROOT/build/qemu-runtime.log"
    QEMU_RC=0

    rm -f "$QEMU_LOG"

    qemu-system-x86_64 \
        -m 512M \
        -cdrom "$PROJECT_ROOT/XyrisOS.iso" \
        -boot d \
        -serial stdio \
        -display none \
        -no-reboot \
        -no-shutdown >"$QEMU_LOG" 2>&1 &
    QEMU_PID=$!

    REQUIRED_MARKERS=(
        "Kernel Ready"
        "Syscall Test: Open"
        "Syscall Test: Read"
        "Syscall Test: Close"
        "User Test: Address Space Cleanup"
        "THREAD A FINISHED"
        "THREAD B FINISHED"
    )

    QEMU_STATUS=FAIL

    for ((i=0; i<45; i++)); do
        if [[ -f "$QEMU_LOG" ]]; then
            ALL_FOUND=1

            for marker in "${REQUIRED_MARKERS[@]}"; do
                if [[ ! -s "$QEMU_LOG" ]] || ! grep -qF "$marker" "$QEMU_LOG"; then
                    ALL_FOUND=0
                    break
            fi

            done

            if [[ "$ALL_FOUND" -eq 1 ]]; then
                QEMU_STATUS=PASS
                break
            fi
        fi

        sleep 1
    done

    kill "$QEMU_PID" 2>/dev/null || true
    wait "$QEMU_PID" 2>/dev/null || true


    if grep -q "\[FAIL\]" "$QEMU_LOG"; then
        echo "ERROR: kernel reported one or more test failures."
        cat "$QEMU_LOG"
        exit 1
    fi

    if grep -q "PMM FREE REJECT" "$QEMU_LOG"; then
        echo "ERROR: PMM rejected one or more physical-page frees."
        cat "$QEMU_LOG"
        exit 1
    fi

    if grep -q "Kernel Ready" "$QEMU_LOG" && \
       grep -q "Syscall Test: Open" "$QEMU_LOG" && \
       grep -q "Syscall Test: Read" "$QEMU_LOG" && \
       grep -q "Syscall Test: Close" "$QEMU_LOG" && \
       grep -q "User Test: Address Space Cleanup" "$QEMU_LOG" && \
       grep -q "THREAD A FINISHED" "$QEMU_LOG" && \
       grep -q "THREAD B FINISHED" "$QEMU_LOG"; then
        echo "PASS: QEMU boot, kernel tests, user cleanup, and scheduler completion markers detected."
        QEMU_STATUS=PASS
    else
        echo "ERROR: required QEMU runtime markers were not detected."
        echo "QEMU exit status: $QEMU_RC"
        cat "$QEMU_LOG"
        exit 1
    fi
else
    echo "BLOCKED: qemu-system-x86_64 or a generated ISO is unavailable."
    QEMU_STATUS=BLOCKED
fi

echo ""
echo "=========================================="
echo "Validation summary"
echo "=========================================="
echo "Kernel build: PASS"
echo "Unresolved symbols: PASS"
echo "Simulator/CTest: PASS"
echo "ISO generation: $ISO_STATUS"
echo "QEMU runtime: $QEMU_STATUS"

if [[ "$ISO_STATUS" == BLOCKED || "$QEMU_STATUS" == BLOCKED ]]; then
    echo ""
    echo "Source/build validation is complete."
    echo "Runtime acceptance remains environment-dependent until the missing host tools are installed."
fi
