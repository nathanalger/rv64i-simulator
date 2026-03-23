#pragma once
#include "Utility.h"
#include "Processor.h"

#define DEBUG_HISTORY_SIZE 8192

#define DEBUG_LOG(msg) Debug::log(__func__, msg)

#define TRACE_BEGIN()                      \
   if (Debug::trace() && Debug::enabled()) \
   {                                       \
      Debug::begin(__func__);

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
   static bool trace();
   static void enable();
   static void enable_trace();
   static void disable();
   static bool set(bool enabled);

   static void log(const char *caller, const char *message);
   static void begin(const char *caller);
   static void end();
   static void recordChar(char c);
   static void recordString(const char *s);
   static void dump();

   static void printDebugRegisters(const Processor &cpu)
   {
      recordString("--- REGISTER DUMP ---\n");
      for (int i = 0; i < 32; i++)
      {
         recordString("x");
         recordString(Utility::int64_to_hex(cpu.registers[i]));
         recordChar('\n');
      }
   }

private:
   static bool debug;
   static bool tr;

   // Circular buffer members
   static char history[DEBUG_HISTORY_SIZE];
   static unsigned int head;
   static bool wrapped;
};