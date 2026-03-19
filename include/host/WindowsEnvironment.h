#pragma once
#include "Environment.h"
#include "Processor.h"
#include "IODevice.h"

class WindowsEnvironment : public Environment
{
public:
   void handle_ecall(Processor &cpu) override;
   void handle_ebreak(Processor &cpu) override;

   ~WindowsEnvironment() override = default;
};