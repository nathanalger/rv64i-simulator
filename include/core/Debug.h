#pragma once

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

private:
   static bool debug;
};

#define DEBUG_LOG(msg) Debug::log(__func__, msg)
#define DEBUG_BEGIN()    \
   if (Debug::enabled()) \
   {                     \
      Debug::begin(__func__);
#define DEBUG_END() \
   Debug::end();    \
   }