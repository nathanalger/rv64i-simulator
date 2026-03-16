#include "Memory.h"

Memory::Memory(size_t size) {
   memory.resize(size);
}

uint8_t Memory::readByte(uint64_t address) {
   return memory[address];
}
uint8_t Memory::writeByte(uint64_t address, uint8_t value) {
   memory[address] = value;
}

uint32_t Memory::readWord(uint64_t address) {
    return  (uint32_t)memory[address + 0] |
           ((uint32_t)memory[address + 1] << 8) |
           ((uint32_t)memory[address + 2] << 16) |
           ((uint32_t)memory[address + 3] << 24);
}

uint64_t Memory::writeDouble(uint64_t address, uint64_t value) {
    memory[address + 0] = value & 0xFF;
    memory[address + 1] = (value >> 8) & 0xFF;
    memory[address + 2] = (value >> 16) & 0xFF;
    memory[address + 3] = (value >> 24) & 0xFF;
    memory[address + 4] = (value >> 32) & 0xFF;
    memory[address + 5] = (value >> 40) & 0xFF;
    memory[address + 6] = (value >> 48) & 0xFF;
    memory[address + 7] = (value >> 56) & 0xFF;

    return value;
}

uint64_t Memory::readDouble(uint64_t address) {
    return  (uint64_t)memory[address + 0] |
           ((uint64_t)memory[address + 1] << 8) |
           ((uint64_t)memory[address + 2] << 16) |
           ((uint64_t)memory[address + 3] << 24) |
           ((uint64_t)memory[address + 4] << 32) |
           ((uint64_t)memory[address + 5] << 40) |
           ((uint64_t)memory[address + 6] << 48) |
           ((uint64_t)memory[address + 7] << 56);
}