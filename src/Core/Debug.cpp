#include "Debug.h"
#include "IODevice.h"

bool Debug::debug = false;

bool Debug::enabled()
{
   return debug;
}

void Debug::enable()
{
   debug = true;
}

void Debug::disable()
{
   debug = false;
}

bool Debug::set(bool enabled)
{
   debug = enabled;
   return enabled;
}

void Debug::log(const char *caller, const char *message)
{
   if (!debug || !io)
      return;

   io->writeChar('[');
   io->writeString(caller);
   io->writeString("] ");
   io->writeString(message);
   io->writeChar('\n');
}

void Debug::begin(const char *caller)
{
   if (!debug || !io)
      return;

   io->writeChar('[');
   io->writeString(caller);
   io->writeString("] ");
}

void Debug::end()
{
   if (!debug || !io)
      return;

   io->writeChar('\n');
}