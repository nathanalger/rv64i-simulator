#include "InjectionLoader.h"
#include "Debug.h"
#include "IODevice.h"
#include <fstream>
#include <cstdint>

uint64_t InjectionLoader::load(Bus &bus, uint64_t start_address)
{
   std::ifstream file(filename, std::ios::binary);

   if (!file)
   {
      io->writeString("Failed to open file.");
      exit(1);
   }

   // Read file byte by byte and write to the Bus
   char byte;
   uint64_t offset = 0;
   while (file.get(byte))
   {
      // We write to start_address + offset (e.g., 0x80000000, 0x80000001...)
      bus.writeByte(start_address + offset, static_cast<uint8_t>(byte));
      offset++;
   }

   DEBUG_BEGIN()
   io->writeString("Loaded ");
   io->writeInt(offset);
   io->writeString(" bytes into memory starting at ");
   io->writeInt(start_address);
   io->writeString(".\n");
   DEBUG_END()

   return offset;
}