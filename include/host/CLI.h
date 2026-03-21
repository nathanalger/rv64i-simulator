#pragma once
#include <string>
#include <cstddef>

struct CLI
{
   char *filename;
   size_t memory_kb = 1024;
   bool debug_enabled = false;
   bool valid = false;
};

// Function declaration
CLI parseCommandLine(int argc, char *argv[]);