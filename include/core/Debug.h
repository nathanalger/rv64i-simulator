#pragma once
#include "Utility.h"
#include "Processor.h"

#define DEBUG_HISTORY_SIZE 8192

#define DEBUG_LOG(msg) Debug::log(__func__, msg)

// Macros now ALWAYS execute the block so history is captured
#define TRACE_BEGIN() \
   {                  \
      Debug::begin(__func__);

#define DEBUG_BEGIN() \
   {                  \
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

   // Core Recording
   static void recordChar(char c);
   static void recordString(const char *s);
   static void dump();
   static void enableTrace()
   {
      tr = true;
   };

   // High-level loggers (Always record, conditionally print)
   static void log(const char *caller, const char *message);
   static void begin(const char *caller);
   static void end();

   // Helpers to replace direct 'io' calls inside debug blocks
   static void writeString(const char *s);
   static void writeInt(uint64_t val);
   static void writeHex(uint64_t val);

   static void printDebugRegisters(const Processor &cpu)
   {
      DEBUG_BEGIN()
      writeString("--- REGISTER DUMP ---\n");
      for (int i = 0; i < 32; i++)
      {
         writeString("x");
         writeInt(i);
         writeString(": ");
         writeHex(cpu.registers[i]);
         writeString("\n");
      }
      DEBUG_END()
   }

private:
   static bool debug;
   static char history[DEBUG_HISTORY_SIZE];
   static unsigned int head;
   static bool wrapped;
   static bool tr;
};