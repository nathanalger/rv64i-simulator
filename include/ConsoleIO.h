#pragma once
#include "IO.h"
#include <iostream>

class ConsoleIO : public IO
{
public:
   void writeChar(char c) override
   {
      std::cout << c;
   }
};