#pragma once

class Processor;

class Environment
{
public:
   virtual void handle_ecall(Processor &cpu) = 0;
   virtual void handle_ebreak(Processor &cpu) = 0;

   virtual ~Environment() = default;
};