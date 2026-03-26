#pragma once
#include <string>
#include <cstddef>

struct CLI
{
   char *filename;
   size_t memory_kb = 50 * 1024;
   bool debug_enabled = false;
   bool trace_enabled = false;
   bool valid = false;
};

CLI parseCommandLine(int argc, char *argv[]);