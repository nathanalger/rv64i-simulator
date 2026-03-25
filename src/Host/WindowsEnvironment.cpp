#include "WindowsEnvironment.h"
#include "Debug.h"
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#include <intrin.h>
#endif

#include "WindowsEnvironment.h"
#include <string>
#include <iostream>

#define SYS_READ 63
#define SYS_WRITE 64
#define SYS_EXIT 93
#define SYS_BRK 214

void WindowsEnvironment::handle_ecall(Processor &cpu)
{
   uint64_t syscall_num = cpu.registers[17];
   uint64_t arg0 = cpu.registers[10];
   uint64_t arg1 = cpu.registers[11];
   uint64_t arg2 = cpu.registers[12];

   switch (syscall_num)
   {
   case SYS_WRITE:
   {
      for (uint64_t i = 0; i < arg2; ++i)
      {
         char c = static_cast<char>(cpu.bus.readByte(arg1 + i));
         if (io)
            io->writeChar(c);
      }
      cpu.write_reg(10, arg2);
      break;
   }

   case SYS_READ:
   {
      std::string input;
      std::getline(std::cin, input);
      uint64_t bytes_to_copy = (arg2 < input.length()) ? arg2 : input.length();

      for (uint64_t i = 0; i < bytes_to_copy; ++i)
      {
         cpu.bus.writeByte(arg1 + i, input[i]);
      }
      cpu.write_reg(10, bytes_to_copy);
      break;
   }

   case SYS_BRK:
   {
      // TODO
      // This is used for malloc. For now, we return a fake heap pointer
      // or the requested address if it's within bounds.
      // A simple "always succeed" dummy for now:
      cpu.write_reg(10, arg0 == 0 ? 0x400000 : arg0);
      break;
   }

   case SYS_EXIT:
   {
      int exit_code = static_cast<int>(arg0);
      if (io)
      {
         io->writeString("\n[OS] Program exited with code: ");
         io->writeInt(exit_code);
         io->writeString("\n");
      }
      std::exit(exit_code);
      break;
   }

   default:
      if (io)
      {
         io->writeString("[WARNING] Unhandled ECALL: ");
         io->writeInt(syscall_num);
         io->writeString("\n");
      }
      break;
   }
}

void WindowsEnvironment::handle_ebreak(Processor &cpu)
{
   if (io && Debug::enabled())
   {

      io->writeString("[EBREAK] Breakpoint hit at PC: ");
      io->writeInt(cpu.program_counter);
      io->writeString("\n");
   }

#ifdef _WIN32
   if (IsDebuggerPresent())
   {
      __debugbreak();
   }
#endif
}