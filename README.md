# RV64I Simulator

This is an OpenSBI compliant, fully featured RISC-V System Simulator. This emulator is capable of running from small compiled RISC-V program to featured operating systems.

This project simulates multiple parts of a full RISC-V computer system, such as:

- Processor
- Memory
- Bus
- UART and IO
- And More

In its current state, this simulator is currently OpenSBI compliant. The goal is to run linux systems in the future.

## Overview

This project is intended as a project to refine my skills in C++ as well as my understanding of RISC-V operations and operating systems.

Core features include:
- 32 general-purpose 64-bit registers
- Protected RAM
- Instruction registry map for easy instruction implementation
- Standard UART implementation
- Standard CSR Registers with Privilege States
- Modular environment handling

## Build Variants

This project has two different build variants, `Host` and `Core`.

**Host** is meant to be a simulator run inside of an operating system that has a terminal and system functions to aid in initialization and interaction with the simulator. This uses standard io libraries to output to a console, as well as to inject code into memory on initialization. It additionally has a CLI interface for debugging and memory management. Host can be used like so:

```bash
./rv64i <binary_file> [--memory kb] [--debug]
```

**Core** is a bare-bones implementation of the simulator. It is still entirely capable of simulation, however it is independent of any standard libraries. This is intended to, at some point, be run on top of a kernel, with no true operating system, so that a RISC-V operating system could be simulated at a low level.

### Separation of files

For any files that require stdlib provided by the OS will be stored separately. 

**Header files** are separated into folders by name. In `include/core` are header files that are integral and are included in both Core and Host. 

**Code files** are a bit different. When compiling core, it will include `src/Core` as well as `src/Instructions`. When compiling host, it will additionally include `src/Host`.

## Recommended IDE: VS Code

This project is best used with **Visual Studio Code**.

The repository includes preconfigured:
- Build tasks (`tasks.json`)
- Project settings

These are designed to make compiling and running the simulator quick and consistent without additional setup.

### Why VS Code?

- One-click build and run using built-in tasks
- Consistent environment across different systems
- Integrated terminal and debugging support

### Easy Building

Simply build using `Ctrl + Shift + B`. The output file will be in `dist/`.

## Adding Instructions

Check out:

**[Adding Instructions Guide](docs/adding-instructions.md)**

This explains:
- How instructions are decoded
- How to implement new operations
- Where to integrate them in the execution pipeline

## Building

The build command can be found in `.vscode/tasks.json`. You will find that a build produces two different binaries.

## AI Disclosure

I am a solo developer and a student, and Artificial Intelligence has been a massive help in learning specific system functions that I did not previously know were necessary. While the main RISC-V interpreter and core functions were implemented by hand, CSRs and some RISC-V C extension handling was implemented with the assistance of AI, as doing it by hand would take an insane amount of time. Leveraging AI for these repetitive tasks allowed me to focus my efforts on system design and OpenSBI compliance.