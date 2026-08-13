# XyrisOS Memory Management

## Overview

The XyrisOS memory subsystem provides physical memory management,
virtual memory management, kernel heap allocation, address space
management, memory protection, and memory subsystem testing.

The subsystem is designed for x86_64 architecture and uses four-level
paging with a higher half direct mapping (HHDM) for physical memory access.

---

# Physical Memory Manager (PMM)

The Physical Memory Manager manages physical memory pages using a
bitmap-based page allocator.

## Features

- Physical page allocation
- Physical page freeing
- Multi-page allocation support
- Bitmap-based memory tracking
- Memory usage statistics

## Statistics

The PMM provides memory statistics including:

- Total pages
- Free pages
- Used pages

These statistics are used for validating allocations and detecting
memory leaks during testing.

---

# Higher Half Direct Map (HHDM)

The Higher Half Direct Map provides a direct virtual mapping of physical
memory.

It allows the kernel to access physical memory using virtual addresses
without requiring manual mapping for every physical page.

## Features

- Physical-to-virtual address conversion
- Virtual-to-physical address conversion
- Kernel access to physical memory

---

# Virtual Memory Manager (VMM)

The Virtual Memory Manager manages x86_64 virtual memory using
four-level paging structures:

- PML4
- Page Directory Pointer Table (PDPT)
- Page Directory (PD)
- Page Table (PT)

## Features

- Page table walking
- Virtual address mapping
- Page unmapping
- Virtual-to-physical address translation
- Page flag management
- Page protection updates
- Address space creation
- Address space destruction

## Address Space Management

XyrisOS supports independent virtual address spaces.

Implemented operations:

- Create address space
- Initialize page tables
- Destroy address space
- Free user-space page tables

Kernel mappings are shared between address spaces.

---

# Memory Protection Framework

The VMM provides basic page-level memory protection.

Supported page flags include:

- Present pages
- Writable pages
- User/supervisor permissions
- Page protection changes

The system supports changing page permissions using
`vmm_protect_page()`.

---

# Kernel Heap

The kernel heap provides dynamic memory allocation inside the kernel.

## Functions

- `kmalloc()`
- `kcalloc()`
- `krealloc()`
- `kfree()`

The allocator currently uses a bump allocation strategy with
16-byte alignment.

## Tested Features

- Multiple allocations
- Allocation of different sizes
- Memory alignment verification
- Memory freeing

---

# Page Fault Handler

XyrisOS includes a basic x86_64 page fault handler.

## Features

The handler provides diagnostics for:

- Fault address using CR2
- Instruction pointer (RIP)
- Stack pointer (RSP)
- Page fault error code

## Error Analysis

The handler identifies:

- Protection violations
- Non-present pages
- Read/write access type
- User/kernel access
- Instruction fetch faults

Unhandled page faults are reported through kernel panic diagnostics.

---

# Memory Testing

The memory subsystem includes automated validation tests.

## PMM Tests

- Page allocation test
- Allocation statistics verification
- Page freeing test
- Multi-page allocation stress test

## Heap Tests

- Multiple allocation test
- Alignment verification
- Free operation test

## VMM Tests

- Page mapping test
- Address translation test
- Page flag retrieval test
- Page protection test
- Updated permission verification
- Page unmapping test
- Invalid translation test
- Double unmap handling
- Protection of missing pages

## Address Space Tests

- Address space creation
- Address space destruction
- Address space stress testing
- Memory leak verification

---

# Current Status

The XyrisOS memory subsystem is complete for the current development
phase.

Implemented components:

- Physical Memory Manager
- Higher Half Direct Map
- Virtual Memory Manager
- Paging subsystem
- Address space abstraction
- Kernel heap allocator
- Memory protection framework
- Memory statistics
- Basic page fault handler
- Memory subsystem test suite