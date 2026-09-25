#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
LOG_FILE="${XYRIS_E2E_LOG:-$BUILD_DIR/e2e-qemu.log}"
TIMEOUT="${XYRIS_E2E_TIMEOUT:-300}"

cd "$PROJECT_ROOT"

if ! command -v xorriso >/dev/null 2>&1; then
    echo "BLOCKED: xorriso is required for M10 7.10 end-to-end testing."
    exit 2
fi
if ! command -v qemu-system-x86_64 >/dev/null 2>&1; then
    echo "BLOCKED: qemu-system-x86_64 is required for M10 7.10 end-to-end testing."
    exit 2
fi

rm -rf "$BUILD_DIR"
"$PROJECT_ROOT/scripts/build.sh"
"$PROJECT_ROOT/scripts/iso.sh"

rm -f "$LOG_FILE"
mkdir -p "$(dirname "$LOG_FILE")"

qemu-system-x86_64 \
    -machine q35 \
    -m 512M \
    -cdrom "$PROJECT_ROOT/XyrisOS.iso" \
    -boot d \
    -serial file:"$LOG_FILE" \
    -display none \
    -no-reboot \
    -no-shutdown &
QEMU_PID=$!
cleanup() {
    kill "$QEMU_PID" 2>/dev/null || true
    wait "$QEMU_PID" 2>/dev/null || true
}
trap cleanup EXIT

REQUIRED_MARKERS=(
    "Kernel Ready"
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

for ((elapsed=0; elapsed<TIMEOUT; elapsed++)); do
    if [[ -f "$LOG_FILE" ]]; then
        complete=1
        for marker in "${REQUIRED_MARKERS[@]}"; do
            if ! grep -qF "$marker" "$LOG_FILE"; then
                complete=0
                break
            fi
        done
        if [[ "$complete" -eq 1 ]]; then
            break
        fi
    fi
    sleep 1
done

if [[ ! -s "$LOG_FILE" ]]; then
    echo "FAIL: QEMU produced no serial output."
    exit 1
fi
if grep -qF "[FAIL]" "$LOG_FILE"; then
    echo "FAIL: kernel reported a test failure."
    cat "$LOG_FILE"
    exit 1
fi
if grep -qF "PMM FREE REJECT" "$LOG_FILE"; then
    echo "FAIL: physical-page free rejection detected."
    cat "$LOG_FILE"
    exit 1
fi

missing=()
for marker in "${REQUIRED_MARKERS[@]}"; do
    grep -qF "$marker" "$LOG_FILE" || missing+=("$marker")
done
if ((${#missing[@]})); then
    echo "FAIL: end-to-end acceptance markers missing:"
    printf ' - %s\n' "${missing[@]}"
    cat "$LOG_FILE"
    exit 1
fi

echo "M10 7.10 end-to-end test: PASS"
echo "Boot -> kernel -> syscall -> userspace -> drivers -> scheduler lifecycle: PASS"
echo "Serial log: $LOG_FILE"
