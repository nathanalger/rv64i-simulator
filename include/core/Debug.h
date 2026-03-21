#pragma once
#include "Utility.h"
#include "Processor.h"

#define DEBUG_LOG(msg) Debug::log(__func__, msg)

#define DEBUG_BEGIN()    \
   if (Debug::enabled()) \
   {                     \
      Debug::begin(__func__);

#define DEBUG_END() \
   Debug::end();    \
   }

class Debug
{
public:
   static bool enabled();
   static void enable();
   static void disable();
   static bool set(bool enabled);
   static void log(const char *caller, const char *message);
   static void begin(const char *caller);
   static void end();
   static void printDebugRegisters(const Processor &cpu)
   {
      DEBUG_LOG("Final register values after execution:");
      for (int i = 0; i < 32; i++)
      {
         DEBUG_BEGIN()
         io->writeString("x");
         io->writeInt(i);
         io->writeString(" - ");
         io->writeString(Utility::int64_to_hex(cpu.registers[i]));
         DEBUG_END()
      }
   }

private:
   static bool debug;
};