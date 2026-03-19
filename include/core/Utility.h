#include <cstdint>

class Utility
{
public:
   /**
    * Unsafe for use outside of debugging. Uses a shared
    * array, will break in asynchronous situations
    */
   static char *int64_to_hex(uint64_t value);

private:
   static char hexbuffer[19];
};