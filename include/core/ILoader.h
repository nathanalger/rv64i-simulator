#pragma once
#include "Memory.h"

class ILoader
{
public:
   /**
    * Returns the next free address after loading the
    * program into memory. Returns 0 upon failure.
    */
   virtual uint64_t load(Memory &mem) = 0;
   virtual ~ILoader() = default;
};