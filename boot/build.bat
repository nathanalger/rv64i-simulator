@echo off
echo ===================================================
echo Building boot.s for RISC-V Bare-Metal Emulator...
echo ===================================================

:: Check if the assembly file actually exists
if not exist boot.s (
    echo ERROR: boot.s not found in the current directory!
    pause
    exit /b 1
)

echo [1/3] Compiling to ELF...
riscv-none-elf-gcc -march=rv64i_zicsr -mabi=lp64 -nostdlib -Ttext=0 boot.s -o boot.elf

:: Check if the compiler threw any errors
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo BUILD FAILED! Please check your assembly syntax.
    pause
    exit /b %ERRORLEVEL%
)

echo [2/3] Extracting raw binary (boot.bin)...
riscv-none-elf-objcopy -O binary boot.elf boot.bin

echo [3/3] Generating disassembly (boot.asm)...
riscv-none-elf-objdump -d boot.elf > boot.asm

echo.
echo SUCCESS! boot.bin is ready for your emulator.
pause