#pragma once
#include "Memory.h"
#include "Bus.h"

class ILoader
{
public:
   /**
    * Returns the next free address after loading the
    * program into memory. Returns 0 upon failure.
    */
   virtual uint64_t load(Bus &bus, uint64_t start_address) = 0;
   virtual ~ILoader() = default;
};