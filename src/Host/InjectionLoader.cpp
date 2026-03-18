#include "InjectionLoader.h"
#include "Debug.h"
#include "IODevice.h"
#include <fstream>
#include <cstdint>

uint64_t InjectionLoader::load(Memory &mem)
{
   std::ifstream file(filename, std::ios::binary);

   if (!file)
      return 0;

   uint64_t address = 0;

   char byte;
   while (file.read(&byte, 1))
   {
      mem.writeByte(address, static_cast<uint8_t>(byte));
      address++;
   }

   DEBUG_BEGIN()
   io->writeString("Loaded ");
   io->writeInt(address);
   io->writeString(" bytes into memory.");
   DEBUG_END()

   return address;
}