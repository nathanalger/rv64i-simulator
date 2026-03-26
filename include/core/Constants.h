#pragma once
#include <stddef.h>
#include <stdint.h>

constexpr uint64_t MSTATUS_MIE = (1ULL << 3);
constexpr uint64_t MSTATUS_MPIE = (1ULL << 7);
constexpr uint64_t MSTATUS_MPP_MASK = (3ULL << 11);
constexpr uint64_t MSTATUS_MPP_SHIFT = 11;

constexpr uint64_t SSTATUS_SIE = (1ULL << 1);
constexpr uint64_t SSTATUS_SPIE = (1ULL << 5);
constexpr uint64_t SSTATUS_SPP_MASK = (1ULL << 8);
constexpr uint64_t SSTATUS_SPP_SHIFT = 8;

constexpr uint64_t SSTATUS_MASK = SSTATUS_SIE | SSTATUS_SPIE | SSTATUS_SPP_MASK;

constexpr uint64_t SATP_MODE_SHIFT = 60;
constexpr uint64_t SATP_MODE_MASK = (0xFULL << SATP_MODE_SHIFT);
constexpr uint64_t SATP_MODE_BARE = 0;
constexpr uint64_t SATP_MODE_SV39 = 8;

constexpr uint64_t SATP_PPN_MASK = 0xFFFFFFFFFFFULL;

constexpr uint64_t PAGE_SIZE = 4096;

constexpr uint64_t PTE_V = (1 << 0);
constexpr uint64_t PTE_R = (1 << 1);
constexpr uint64_t PTE_W = (1 << 2);
constexpr uint64_t PTE_X = (1 << 3);
constexpr uint64_t PTE_U = (1 << 4);

constexpr uint64_t PTE_PPN_SHIFT = 10;
constexpr uint64_t PTE_PPN_MASK = 0xFFFFFFFFFFFULL;

constexpr uint64_t CLINT_BASE = 0x02000000;
constexpr uint64_t CLINT_MTIMECMP = CLINT_BASE + 0x4000;
constexpr uint64_t CLINT_MTIME = CLINT_BASE + 0xBFF8;

constexpr uint64_t MIP_MTIP = (1ULL << 7);
constexpr uint64_t MIE_MTIE = (1ULL << 7);