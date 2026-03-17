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

Typical build (example using `g++`):

```bash
g++ -Wall -Wextra -std=c++17 src/main.cpp -o dist/rv64i
```