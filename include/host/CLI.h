#pragma once
#include <string>
#include <cstddef>
#include <cstdint>

enum class SystemType
{
   Default,
   OpenSBI
};

struct CLI
{
   char *filename;
   uint32_t memory_kb = 531072;
   SystemType systemType = SystemType::Default;
   bool debug_enabled = false;
   bool trace_enabled = false;
   bool valid = false;
};

CLI parseCommandLine(int argc, char *argv[]);