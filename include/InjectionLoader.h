#pragma once
#include "Memory.h"
#include "ILoader.h"
#include <string>

/**
 * Takes a file and reads its contents, then injects it
 * directly into memory and runs it.
 *
 * This is meant for hosted enviroments only that support
 * the standard IO libraries.
 */
class InjectionLoader : public ILoader
{
public:
   InjectionLoader(const std::string &file) : filename(file) {}

   /**
    * Loads information byte-by-byte into memory starting at address 0.
    */
   bool load(Memory &mem) override;

private:
   std::string filename;
};