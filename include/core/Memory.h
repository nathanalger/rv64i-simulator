#pragma once
#include <stddef.h>
#include <stdint.h>

/**
 * Simulates memory module and controllers. Protects the memory from the simulator by utilizing getter-setter architecture.
 */
class Memory
{
private:
   // Memory is protected from processor
   uint8_t *memory;
   size_t size;

public:
   // Byte-Addressable Access

   /**
    * Writes a byte (8 bits) of data into the specified address.
    * Addresses are byte addressable.
    */
   uint8_t writeByte(uint64_t address, uint8_t value);

   /**
    * Reads the byte (8 bits) of data from the specified address.
    * Addresses are byte addressable.
    */
   uint8_t readByte(uint64_t address);

   // Word-Addressable Access

   /**
    * Writes a half-word (16 bits) of data into the specified address.
    * Addresses are byte addressable.
    */
   uint16_t writeHalf(uint64_t address, uint16_t value);

   /**
    * Reads the half-word (16 bits) of data from the specified address.
    * Addresses are byte addressable.
    */
   uint16_t readHalf(uint64_t address);

   /**
    * Writes a word (32 bits) of data into the specified address.
    * Addresses are byte addressable.
    */
   uint32_t writeWord(uint64_t address, uint32_t value);

   /**
    * Reads the word (32 bits) of data from the specified address.
    * Addresses are byte addressable.
    */
   uint32_t readWord(uint64_t address);

   // Double-Addressable Access

   /**
    * Writes a double-word (64 bits) of data into the specified address.
    * Addresses are byte addressable.
    */
   uint64_t writeDouble(uint64_t address, uint64_t value);

   /**
    * Reads the double-word (64 bits) of data from the specified address.
    * Addresses are byte addressable.
    */
   uint64_t readDouble(uint64_t address);

   uint64_t getSize();

   // Initializer
   Memory(size_t size);
   ~Memory();

   // Disable copying
   Memory(const Memory &) = delete;
   Memory &operator=(const Memory &) = delete;
};