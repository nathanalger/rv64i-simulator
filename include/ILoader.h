#pragma once
#include "Memory.h"

class ILoader
{
public:
   virtual bool load(Memory &mem) = 0;
   virtual ~ILoader() = default;
};