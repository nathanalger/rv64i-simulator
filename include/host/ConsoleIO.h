#pragma once
#include "IO.h"
#include <iostream>

class ConsoleIO : public IO
{
public:
   void writeChar(char c) override
   {
      std::cout << c << std::flush;
   }
   void pause() override;
   void resume() override;
};