# RV64I Simulator

A lightweight C++ simulator for the **RISC-V RV64I instruction set**.  
This project emulates a basic 64-bit processor, including registers, memory, instruction decoding, and execution.


## Overview

This simulator is designed to help understand how a CPU works at a low level by implementing the **fetch–decode–execute cycle** for RV64I instructions.

Core features include:
- 32 general-purpose 64-bit registers
- Simulated RAM
- Instruction decoding system
- Execution engine for core RV64I instructions
- Debug logging support


## Architecture

The simulator is organized into a few key components:

- **Processor**
  - Holds register state and program counter
- **Memory**
  - Simulates RAM and supports read/write operations
- **Interpreter / Decoder**
  - Converts raw instructions into structured formats
- **Instruction Execution**
  - Executes decoded instructions like `ADD`, `SUB`, `LW`, `SW`, etc.


## Getting Started

To build and run the simulator, follow the setup guide:

**[Getting Started Guide](docs/getting-started.md)**

This includes:
- Build instructions
- Project structure overview
- How to run your first program

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

## Build

The build command can be found in `.vscode/tasks.json`. You will find that a build produces two different binaries.

### RV64I Core

RV64I Core is the binaries that are intended to run entirely independent of standard libraries. This will become important when executing the binaries with no operating system.

### RV64I Host

RV64I Host additionally includes binaries that make it executable on a machine with an operating system. It provides some standard libraries, mostly for IO. 

### Separation of files

For any files that require IO provided by the OS will be stored separately. 

**Header files** are separated into folders by name. In `include/core` are header files that are integral and are included in both Core and Host. 

**Code files** are a bit different. When compiling core, it will include `src/Core` as well as `src/Instructions`. When compiling host, it will additionally include `src/Host`.