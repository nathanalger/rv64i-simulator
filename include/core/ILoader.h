// ILoader.h
#pragma once
#include <string>
#include <cstdint>
#include "Bus.h"

class ILoader
{
public:
   virtual uint64_t load(Bus &bus, uint64_t start_address, const char *filename) = 0;
   virtual ~ILoader() = default;
};