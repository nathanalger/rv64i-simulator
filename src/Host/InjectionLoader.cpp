#include "InjectionLoader.h"
#include "Debug.h"
#include "IODevice.h"
#include <fstream>

uint64_t InjectionLoader::load(Bus &bus, uint64_t start_address, const char *filename)
{
   std::ifstream file(filename, std::ios::binary);

   if (!file)
   {
      // Use your IO device to report the specific file that failed
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
   io->writeString("Successfully injected '");
   io->writeString(filename);
   io->writeString("' (");
   io->writeInt(offset);
   io->writeString(" bytes) at ");
   io->writeInt(start_address);
   io->writeString("\n");
   DEBUG_END()

   return offset;
}