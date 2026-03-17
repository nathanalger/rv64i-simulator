#pragma once
#include <iostream>

class Debug
{
public:
   static bool enabled();
   static void enable();
   static void disable();
   static bool set(bool enabled);
   static void log(const std::string &caller, const std::string &message);

private:
   static bool debug;
};

#define DEBUG_LOG(msg) Debug::log(__func__, msg)