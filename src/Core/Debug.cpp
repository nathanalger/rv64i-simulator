#include "Debug.h"
#include "IODevice.h"

bool Debug::debug = false;
bool Debug::tr = false;
char Debug::history[DEBUG_HISTORY_SIZE] = {0};
unsigned int Debug::head = 0;
bool Debug::wrapped = false;

void Debug::recordChar(char c)
{
   if (!tr)
      return;

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
   if (!tr || !s)
      return;

   while (*s)
      recordChar(*s++);
}

void Debug::writeString(const char *s)
{
   if (!tr)
      return;

   recordString(s);
   if (debug && io)
      io->writeString(s);
}

void Debug::writeInt(uint64_t val)
{
   recordString(Utility::int64_to_hex(val));
   if (debug && io)
      io->writeInt(val);
}

void Debug::writeHex(uint64_t val)
{
   const char *hex = Utility::int64_to_hex(val);
   recordString(hex);
   if (debug && io)
      io->writeString(hex);
}

void Debug::log(const char *caller, const char *message)
{
   begin(caller);
   writeString(message);
   end();
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
   if (debug)
   {
      io->writeChar('\n');
   }
}

void Debug::dump()
{
   if (!io)
      return;
   io->writeString("\n*** BLACK BOX ***\n");
   if (wrapped)
   {
      for (unsigned int i = head; i < DEBUG_HISTORY_SIZE; i++)
         io->writeChar(history[i]);
   }
   for (unsigned int i = 0; i < head; i++)
      io->writeChar(history[i]);
   io->writeString("\n*** END OF DUMP ***\n");
}

bool Debug::enabled() { return debug; }
void Debug::enable() { debug = true; }
void Debug::disable() { debug = false; }