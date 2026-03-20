#include <cstdint>

// mstatus bit definitions
constexpr uint64_t MSTATUS_MIE = (1ULL << 3);
constexpr uint64_t MSTATUS_MPIE = (1ULL << 7);
constexpr uint64_t MSTATUS_MPP_MASK = (3ULL << 11);
constexpr uint64_t MSTATUS_MPP_SHIFT = 11;

constexpr uint64_t SSTATUS_SIE = (1ULL << 1);
constexpr uint64_t SSTATUS_SPIE = (1ULL << 5);
constexpr uint64_t SSTATUS_SPP_MASK = (1ULL << 8);
constexpr uint64_t SSTATUS_SPP_SHIFT = 8;

constexpr uint64_t SSTATUS_MASK = SSTATUS_SIE | SSTATUS_SPIE | SSTATUS_SPP_MASK;