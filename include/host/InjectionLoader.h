#pragma once
#include "Memory.h"
#include "ILoader.h"
#include "Bus.h"

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
   InjectionLoader() {}
   uint64_t load(Bus &bus, uint64_t start_address, const char *filename) override;
   uint64_t load(Bus &bus, uint64_t start_address, const char *filename, bool staticPath) override;

private:
   char *filename;
};