#include "WindowsEnvironment.h"
#include <cstdlib>

#ifdef _WIN32
#include <windows.h> // For IsDebuggerPresent()
#include <intrin.h>  // For __debugbreak()
#endif

void WindowsEnvironment::handle_ecall(Processor &cpu)
{
   // In RISC-V, the syscall number is placed in x17 (a7)
   uint64_t syscall_num = cpu.registers[17];

   switch (syscall_num)
   {
   case 93: // sys_exit
   {
      // The exit code is placed in x10 (a0)
      int exit_code = static_cast<int>(cpu.registers[10]);

      if (io)
      {
         io->writeString("Program requested exit with code: ");
         io->writeInt(exit_code);
         io->writeString("\n");
      }

      // Gracefully terminate the C++ emulator process
      std::exit(exit_code);
      break;
   }

      // You can add more syscalls here later!
      // Example: case 64: // sys_write

   default:
   {
      if (io)
      {
         io->writeString("[WARNING] Unhandled ECALL: ");
         io->writeInt(syscall_num);
         io->writeString("\n");
      }
      break;
   }
   }
}

void WindowsEnvironment::handle_ebreak(Processor &cpu)
{
   if (io)
   {
      io->writeString("[EBREAK] Breakpoint hit at PC: ");
      io->writeInt(cpu.program_counter);
      io->writeString("\n");
   }

#ifdef _WIN32
   // Only trigger the hardware breakpoint if we are actually debugging the C++ emulator!
   if (IsDebuggerPresent())
   {
      __debugbreak();
   }
#endif
}