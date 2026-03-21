// ILoader.h
#pragma once
#include <string>
#include <cstdint>
#include "Bus.h"

class ILoader
{
public:
   /**
    * Loads a file into the bus at the specified address.
    * Returns the number of bytes loaded, or 0 on failure.
    */
   virtual uint64_t load(Bus &bus, uint64_t start_address, const char *filename) = 0;
   virtual ~ILoader() = default;
};