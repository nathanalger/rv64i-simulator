#include "Debug.h"
#include "IODevice.h"

bool Debug::debug = false;
bool Debug::tr = false;

char Debug::history[DEBUG_HISTORY_SIZE] = {0};
unsigned int Debug::head = 0;
bool Debug::wrapped = false;

void Debug::recordChar(char c)
{
   history[head] = c;
   head++;
   if (head >= DEBUG_HISTORY_SIZE)
   {
      head = 0;
      wrapped = true;
   }
}

void Debug::recordString(const char *s)
{
   if (!s)
      return;
   while (*s)
   {
      recordChar(*s++);
   }
}

void Debug::dump()
{
   if (!io)
      return;

   io->writeString("Black box log dump...\n");

   // If we wrapped, print from head to end first (the oldest data)
   if (wrapped)
   {
      for (unsigned int i = head; i < DEBUG_HISTORY_SIZE; i++)
      {
         io->writeChar(history[i]);
      }
   }

   // Print from start to head (the newest data)
   for (unsigned int i = 0; i < head; i++)
   {
      io->writeChar(history[i]);
   }
}

// Update existing methods to record data ALWAYS
void Debug::log(const char *caller, const char *message)
{
   recordChar('[');
   recordString(caller);
   recordString("] ");
   recordString(message);
   recordChar('\n');

   if (debug && io)
   {
      io->writeChar('[');
      io->writeString(caller);
      io->writeString("] ");
      io->writeString(message);
      io->writeChar('\n');
   }
}

void Debug::begin(const char *caller)
{
   recordChar('[');
   recordString(caller);
   recordString("] ");

   if (debug && io)
   {
      io->writeChar('[');
      io->writeString(caller);
      io->writeString("] ");
   }
}

void Debug::end()
{
   recordChar('\n');
   if (debug && io)
   {
      io->writeChar('\n');
   }
}

bool Debug::enabled()
{
   return debug;
}

bool Debug::trace()
{
   return tr;
}

void Debug::enable()
{
   debug = true;
}

void Debug::enable_trace()
{
   tr = true;
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