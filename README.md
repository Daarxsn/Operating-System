# XyrisOS

XyrisOS is a modular, continuity-centric operating system project designed for experimentation, learning, and systems programming.

The project focuses on building a modular operating system architecture with a strong emphasis on system continuity, reliability, structured components, and controlled interaction between system resources.

---

## Project Components

### Kernel

The kernel contains the core operating system functionality, including:

- Boot initialization
- CPU management
- Memory management
- Process and thread execution
- Interrupt handling
- Graphics and framebuffer support
- Terminal and user interface components
- Kernel object management

### XyrisSim

XyrisSim is the reference simulator for XyrisOS.

It provides a controlled environment for testing and demonstrating operating system concepts without requiring the complete operating system to boot on real hardware.

The simulator supports:

- System initialization
- CPU state simulation
- Memory state simulation
- Display state simulation
- Kernel boot simulation
- System reset operations
- Interactive command execution
- UKOM object simulation

### UKOM Simulator

The simulator includes a Universal Kernel Object Manager (UKOM) simulation layer.

The UKOM simulator supports:

- Process objects
- Thread objects
- Driver objects
- Device objects
- Timer objects
- Event objects
- Resource objects
- Custom objects
- Object creation
- Object lookup
- Object listing
- Reference counting
- Object release
- Object destruction

---

## Testing

XyrisOS uses automated tests built with CMake, Ninja, and CTest.

The simulator test suite currently verifies:

- System initialization
- Memory initialization
- CPU initialization
- Simulator command execution
- Kernel boot behavior
- System reset behavior
- UKOM object creation
- UKOM object lookup
- UKOM reference counting
- UKOM object destruction
- Multiple UKOM object types
- Object limit handling
- Invalid and edge-case input handling

Run the complete simulator test suite with:

```bash
./scripts/simulator-test.sh
## Validation

Run the repository validation pipeline from the project root:

```bash
./scripts/validate.sh
```

The pipeline performs a clean kernel build with warnings treated as errors, an
unresolved-symbol audit, simulator build/CTest execution, and optional ISO/QEMU
runtime validation when `xorriso` and `qemu-system-x86_64` are installed.

A missing host runtime dependency is reported as **BLOCKED**, not silently
represented as a passing boot test.
