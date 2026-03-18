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
   uint64_t load(Memory &mem) override;

private:
   std::string filename;
};