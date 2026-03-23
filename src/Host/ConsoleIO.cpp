#include "ConsoleIO.h"
#include <iostream>
#include <string>

void ConsoleIO::pause()
{
   std::string input;
   std::getline(std::cin, input);
   resume();
}

void ConsoleIO::resume()
{
   std::cout << "[Resuming execution...]\n"
             << std::flush;
}