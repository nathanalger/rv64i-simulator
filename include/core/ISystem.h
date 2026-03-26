#pragma once
#include <stddef.h>
#include <stdint.h>
#include "Processor.h"
#include "Memory.h"

class ISystem
{
public:
   virtual ~ISystem() = default;

   virtual uint64_t getRamBase() const = 0;
   virtual uint64_t payloadLocation() const = 0;

   virtual void boot(Processor &cpu, Memory &mem, Bus &bus) = 0;
};