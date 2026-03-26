CXX = g++
CC  = gcc

HOST_CXXFLAGS = -g -Wall -Wextra -std=c++20
CORE_CXXFLAGS = -g -Wall -Wextra -std=c++20 -ffreestanding -fno-exceptions -fno-rtti

CFLAGS = -g -Wall -Wextra

DIST_DIR = dist
FDT_OBJ_DIR = src/lib/libfdt/temp

COMMON_INCLUDES = -Iinclude/core -Isrc/lib/libfdt -Iinclude/systems

FDT_SRCS = $(wildcard src/lib/libfdt/*.c)
FDT_OBJS = $(patsubst src/lib/libfdt/%.c,$(FDT_OBJ_DIR)/%.o,$(FDT_SRCS))

CORE_LOGIC    = $(wildcard src/core/*.cpp)
INSN_SRCS     = $(wildcard src/instructions/*.cpp)
UTILITY_SRCS  = $(wildcard src/Core/Utility/*.cpp)

HOST_EXE      = $(DIST_DIR)\rv64i.exe
HOST_INCLUDES = $(COMMON_INCLUDES) -Iinclude/host
HOST_ONLY_SRC = src/Host.cpp $(wildcard src/host/*.cpp)
HOST_ALL_SRCS = $(HOST_ONLY_SRC) $(CORE_LOGIC) $(INSN_SRCS) $(UTILITY_SRCS)

CORE_EXE      = $(DIST_DIR)\rv64i-core.exe
CORE_INCLUDES = $(COMMON_INCLUDES)
CORE_ONLY_SRC = src/Core.cpp
CORE_ALL_SRCS = $(CORE_ONLY_SRC) $(CORE_LOGIC) $(INSN_SRCS) $(UTILITY_SRCS)

all: setup $(HOST_EXE) $(CORE_EXE)
	@echo Done!

setup:
	@if not exist "$(DIST_DIR)" (mkdir "$(DIST_DIR)")
	@if not exist "$(FDT_OBJ_DIR)" (mkdir "$(FDT_OBJ_DIR)")
	
$(FDT_OBJ_DIR)/%.o: src/lib/libfdt/%.c
	$(CC) $(CFLAGS) $(COMMON_INCLUDES) -c $< -o $@

# Link host
$(HOST_EXE): $(HOST_ALL_SRCS) $(FDT_OBJS)
	@echo Compiling Host (Hosted Mode)...
	$(CXX) $(HOST_CXXFLAGS) $(HOST_INCLUDES) $^ -o $@

# Link core
$(CORE_EXE): $(CORE_ALL_SRCS) $(FDT_OBJS)
	@echo Compiling Core (Freestanding Mode)...
	$(CXX) $(CORE_CXXFLAGS) $(CORE_INCLUDES) $^ -o $@

clean:
	@echo Cleaning executables...
	@if exist "$(HOST_EXE)" del /q "$(HOST_EXE)"
	@if exist "$(CORE_EXE)" del /q "$(CORE_EXE)"
	@if exist "$(FDT_OBJ_DIR)" rmdir /s /q "$(FDT_OBJ_DIR)"