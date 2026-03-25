#include "InjectionLoader.h"
#include "Debug.h"
#include "IODevice.h"
#include <fstream>

uint64_t InjectionLoader::load(Bus &bus, uint64_t start_address, const char *filename)
{
   std::ifstream file(filename, std::ios::binary);

   if (!file)
   {
      io->writeString("Loader Error: Could not open ");
      io->writeString(filename);
      io->writeString("\n");
      return 0;
   }

   char byte;
   uint64_t offset = 0;
   while (file.get(byte))
   {
      bus.writeByte(start_address + offset, static_cast<uint8_t>(byte));
      offset++;
   }

   DEBUG_BEGIN()
   Debug::writeString("Successfully injected '");
   Debug::writeString(filename);
   Debug::writeString("' (");
   Debug::writeInt(offset);
   Debug::writeString(" bytes) at ");
   Debug::writeInt(start_address);
   Debug::writeString("\n");
   DEBUG_END()

   return offset;
}