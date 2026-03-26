# Compiler
CXX = g++

# --- Host Flags (Standard) ---
HOST_CXXFLAGS = -g -Wall -Wextra -std=c++20

# --- Core Flags (Bare Metal / No-Stdlib) ---
# -ffreestanding: Tells the compiler the standard library may not exist.
# -fno-exceptions: Disables C++ exception handling (requires stdlib support).
# -fno-rtti: Disables dynamic_cast and typeid (requires stdlib support).
CORE_CXXFLAGS = -g -Wall -Wextra -std=c++20 -ffreestanding -fno-exceptions -fno-rtti

# Directory Setup
DIST_DIR = dist

# Common Includes
COMMON_INCLUDES = -Iinclude/core -Isrc/lib/libfdt -Iinclude/systems

# Source Groups
FDT_SRCS      = $(wildcard src/lib/libfdt/*.cpp)
CORE_LOGIC    = $(wildcard src/core/*.cpp)
INSN_SRCS     = $(wildcard src/instructions/*.cpp)
UTILITY_SRCS  = $(wildcard src/Core/Utility/*.cpp)

# --- Target 1: The Host ---
HOST_EXE      = $(DIST_DIR)/rv64i.exe
HOST_INCLUDES = $(COMMON_INCLUDES) -Iinclude/host
HOST_ONLY_SRC = src/Host.cpp $(wildcard src/host/*.cpp)
HOST_ALL_SRCS = $(FDT_SRCS) $(HOST_ONLY_SRC) $(CORE_LOGIC) $(INSN_SRCS) $(UTILITY_SRCS)

# --- Target 2: The Core ---
CORE_EXE      = $(DIST_DIR)/rv64i-core.exe
CORE_INCLUDES = $(COMMON_INCLUDES)
CORE_ONLY_SRC = src/Core.cpp
CORE_ALL_SRCS = $(FDT_SRCS) $(CORE_ONLY_SRC) $(CORE_LOGIC) $(INSN_SRCS) $(UTILITY_SRCS)

all: setup $(HOST_EXE) $(CORE_EXE)
	@echo Done!

setup:
	@if not exist $(DIST_DIR) mkdir $(DIST_DIR)

$(HOST_EXE): $(HOST_ALL_SRCS)
	@echo Compiling Host (Hosted Mode)...
	$(CXX) $(HOST_CXXFLAGS) $(HOST_INCLUDES) $(HOST_ALL_SRCS) -o $(HOST_EXE)

$(CORE_EXE): $(CORE_ALL_SRCS)
	@echo Compiling Core (Freestanding Mode)...
	$(CXX) $(CORE_CXXFLAGS) $(CORE_INCLUDES) $(CORE_ALL_SRCS) -o $(CORE_EXE)

clean:
	@if exist $(DIST_DIR) rmdir /s /q $(DIST_DIR)