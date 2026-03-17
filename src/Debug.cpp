#include "Debug.h"

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

void Debug::log(const std::string &caller, const std::string &message)
{
   if (debug)
      std::cout << "[" << caller << "] " << message << std::endl;
}