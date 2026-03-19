#pragma once

// Forward declare Processor so we don't cause circular includes
class Processor;

class Environment
{
public:
   virtual void handle_ecall(Processor &cpu) = 0;
   virtual void handle_ebreak(Processor &cpu) = 0;

   virtual ~Environment() = default;
};