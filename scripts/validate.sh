#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
SIM_BUILD_DIR="$PROJECT_ROOT/simulator/build"
QEMU_LOG="$BUILD_DIR/qemu-runtime.log"

# Maximum time to wait for QEMU runtime markers.
# Override with: XYRIS_QEMU_TIMEOUT=120 ./scripts/validate.sh
QEMU_TIMEOUT="${XYRIS_QEMU_TIMEOUT:-60}"

if ! [[ "$QEMU_TIMEOUT" =~ ^[0-9]+$ ]] || (( QEMU_TIMEOUT <= 0 )); then
    echo "ERROR: XYRIS_QEMU_TIMEOUT must be a positive integer."
    exit 1
fi

cd "$PROJECT_ROOT"

echo "=========================================="
echo "       XyrisOS Validation Pipeline"
echo "=========================================="

# -----------------------------------------------------------------------------
# [1/5] Clean kernel build
# -----------------------------------------------------------------------------
echo "[1/5] Clean kernel build"
rm -rf "$BUILD_DIR"
cmake -S "$PROJECT_ROOT" -B "$BUILD_DIR" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$PROJECT_ROOT/toolchain/x86_64-toolchain.cmake"
cmake --build "$BUILD_DIR"

# -----------------------------------------------------------------------------
# [2/5] Unresolved-symbol audit
# -----------------------------------------------------------------------------
echo "[2/5] Unresolved-symbol audit"
if nm -u "$BUILD_DIR/kernel.elf" | grep -q .; then
    echo "ERROR: unresolved symbols detected"
    nm -u "$BUILD_DIR/kernel.elf"
    exit 1
fi

echo "PASS: no unresolved kernel symbols"

# -----------------------------------------------------------------------------
# [3/5] Simulator build and tests
# -----------------------------------------------------------------------------
echo "[3/5] Simulator build and tests"
rm -rf "$SIM_BUILD_DIR"
cmake -S "$PROJECT_ROOT/simulator" -B "$SIM_BUILD_DIR" -G Ninja
cmake --build "$SIM_BUILD_DIR"
ctest --test-dir "$SIM_BUILD_DIR" --output-on-failure

# -----------------------------------------------------------------------------
# [4/5] ISO prerequisites / generation
# -----------------------------------------------------------------------------
echo "[4/5] ISO prerequisites"
if command -v xorriso >/dev/null 2>&1; then
    "$PROJECT_ROOT/scripts/iso.sh"
    ISO_STATUS=PASS
else
    echo "BLOCKED: xorriso is not installed; ISO generation not executed."
    ISO_STATUS=BLOCKED
fi

# -----------------------------------------------------------------------------
# [5/5] QEMU runtime
# -----------------------------------------------------------------------------
echo "[5/5] QEMU runtime"

QEMU_STATUS=BLOCKED
QEMU_PID=""

cleanup_qemu() {
    if [[ -n "$QEMU_PID" ]] && kill -0 "$QEMU_PID" 2>/dev/null; then
        kill "$QEMU_PID" 2>/dev/null || true
        wait "$QEMU_PID" 2>/dev/null || true
    fi
}

if command -v qemu-system-x86_64 >/dev/null 2>&1 && [[ -f "$PROJECT_ROOT/XyrisOS.iso" ]]; then
    rm -f "$QEMU_LOG"

    qemu-system-x86_64 \
        -machine q35 \
        -m 512M \
        -device qemu-xhci \
        -cdrom "$PROJECT_ROOT/XyrisOS.iso" \
        -boot d \
        -serial "file:$QEMU_LOG" \
        -display none \
        -no-reboot \
        -no-shutdown &
    QEMU_PID=$!

    handle_signal() {
        cleanup_qemu
        exit 130
    }

    trap cleanup_qemu EXIT
    trap handle_signal INT TERM

    # These markers must be present in the serial log for runtime acceptance.
    # "Kernel Ready" is intentionally not used because the current boot code
    # renders that message to the framebuffer rather than serial output.
    REQUIRED_MARKERS=(
        "Userspace Init: Process Created"
        "Userspace Init: Thread Scheduled"
        "Syscall Test: Open"
        "Syscall Test: Read"
        "Syscall Test: Close"
        "User Test: Address Space Cleanup"
        "Foundation Test: Millisecond Accounting"
        "Driver Test: Keyboard Modifier Decode"
        "Driver Test: Keyboard Pause Sequence"
        "Driver Test: Mouse Packet Event"
        "THREAD A FINISHED"
        "THREAD B FINISHED"
        "Preemption Test: PASS"
    )

    ALL_FOUND=0

    for ((elapsed=0; elapsed<QEMU_TIMEOUT; elapsed++)); do
        if [[ -f "$QEMU_LOG" ]]; then
            if grep -qF "[FAIL]" "$QEMU_LOG"; then
                echo "ERROR: kernel reported one or more test failures."
                cat "$QEMU_LOG"
                exit 1
            fi

            if grep -qF "PMM FREE REJECT" "$QEMU_LOG"; then
                echo "ERROR: PMM rejected one or more physical-page frees."
                cat "$QEMU_LOG"
                exit 1
            fi

            ALL_FOUND=1
            for marker in "${REQUIRED_MARKERS[@]}"; do
                if ! grep -qF "$marker" "$QEMU_LOG"; then
                    ALL_FOUND=0
                    break
                fi
            done

            if [[ "$ALL_FOUND" -eq 1 ]]; then
                QEMU_STATUS=PASS
                break
            fi
        fi

        # Fail fast if QEMU has exited before producing the required markers.
        if ! kill -0 "$QEMU_PID" 2>/dev/null; then
            QEMU_RC=0
            wait "$QEMU_PID" || QEMU_RC=$?
            echo "ERROR: QEMU exited before all runtime markers were detected."
            echo "QEMU exit status: $QEMU_RC"
            [[ -f "$QEMU_LOG" ]] && cat "$QEMU_LOG"
            exit 1
        fi

        sleep 1
    done

    if [[ "$QEMU_STATUS" != "PASS" ]]; then
        echo "ERROR: required QEMU runtime markers were not detected within ${QEMU_TIMEOUT}s."
        echo "QEMU log: $QEMU_LOG"
        [[ -f "$QEMU_LOG" ]] && cat "$QEMU_LOG"
        exit 1
    fi

    cleanup_qemu
    QEMU_PID=""
    trap - EXIT INT TERM

    echo "PASS: QEMU boot, userspace init, kernel tests, user cleanup, and scheduler completion markers detected."
else
    echo "BLOCKED: qemu-system-x86_64 or a generated ISO is unavailable."
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

if [[ "$ISO_STATUS" == "BLOCKED" || "$QEMU_STATUS" == "BLOCKED" ]]; then
    echo ""
    echo "Source/build validation is complete."
    echo "Runtime acceptance remains environment-dependent until the missing host tools are installed."
fi
