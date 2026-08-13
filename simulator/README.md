# XyrisSim

XyrisSim is the reference simulator for XyrisOS. It provides a lightweight environment for developing and testing core operating system concepts without requiring immediate execution on physical hardware.

The simulator is designed to support:

- System initialization
- CPU subsystem testing
- Memory subsystem testing
- Display subsystem testing
- Basic command execution
- Universal Kernel Object Manager (UKOM) simulation
- Automated testing through CTest
- Development and continuous integration workflows

---

## Directory Structure

```text
simulator/
├── include/        # Public header files
├── src/            # Simulator implementation
├── tests/          # Simulator and component tests
├── docs/           # Simulator documentation
└── CMakeLists.txt  # Simulator build configuration