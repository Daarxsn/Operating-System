# XyrisSim Test Report

## 1. Overview

XyrisSim is the reference simulator for XyrisOS.

This document records the automated verification of the simulator, its core subsystems, the simulator command interface, and the Universal Kernel Object Manager (UKOM) integration.

The test suite is executed using CMake, Ninja, and CTest.

---

## 2. Test Execution

The complete simulator test suite can be executed using:

```bash
./scripts/simulator-test.sh