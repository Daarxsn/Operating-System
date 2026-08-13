# XyrisSim Architecture

## Purpose

XyrisSim is the reference simulator for XyrisOS.

Its purpose is to simulate and test core operating system components before they are integrated into the actual operating system.

The simulator provides a lightweight environment for validating system initialization, component state, kernel boot behavior, reset behavior, command execution, and Universal Kernel Object Manager (UKOM) operations.

---

## Simulator Components

XyrisSim is divided into several simulated components:

### System

The system module maintains the overall state of the simulator.

It tracks:

- CPU status
- Memory status
- Display status
- Kernel status
- Available RAM

### CPU

The CPU module simulates processor initialization and maintains the CPU online state.

### Memory

The memory module simulates memory initialization and maintains the configured RAM state.

### Display

The display module simulates display initialization and maintains the display readiness state.

### UKOM Simulator

The UKOM simulator provides a simulated Universal Kernel Object Manager.

It manages a registry of kernel objects and supports:

- Object creation
- Object lookup
- Object existence checking
- Reference counting
- Object release
- Object destruction
- Object listing

Supported simulated object types include:

- Process
- Thread
- Driver
- Device
- Timer
- Event
- Resource
- Custom object

Each object contains:

- A unique object ID
- An object type
- An object state
- A reference count

The UKOM simulator allows the simulator environment to validate object lifecycle behavior before or alongside kernel integration.

### Simulator Core

The simulator core controls:

- Simulator initialization
- Kernel boot simulation
- System reset
- Command execution
- UKOM command processing

---

## Initialization Flow

The simulator follows this initialization sequence:

```text
Start XyrisSim
      |
      v
Initialize System State
      |
      v
Initialize CPU
      |
      v
Initialize Memory
      |
      v
Initialize Display
      |
      v
Initialize UKOM
      |
      v
System Ready