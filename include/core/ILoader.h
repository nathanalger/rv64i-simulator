// ILoader.h
#pragma once
#include <stddef.h>
#include <stdint.h>
#include "Bus.h"

class ILoader
{
public:
   virtual uint64_t load(Bus &bus, uint64_t start_address, const char *filename) = 0;
   virtual ~ILoader() = default;
};