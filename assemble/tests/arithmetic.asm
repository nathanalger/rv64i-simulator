# Setup Base Values for Math
addi t0, zero, 12       # x5  = 12
addi t1, zero, 10       # x6  = 10
addi t2, zero, 2        # x7  = 2  (Used as a shift amount)
addi t3, zero, -8       # x28 = -8 (Used to test sign-extension)

# --- Logical Operations (R-Type) ---
and a0, t0, t1          # x10 = 12 & 10 = 8
or  a1, t0, t1          # x11 = 12 | 10 = 14
xor a2, t0, t1          # x12 = 12 ^ 10 = 6

# --- Shift Operations (R-Type) ---
sll a3, t1, t2          # x13 = 10 << 2 = 40
srl a4, t1, t2          # x14 = 10 >> 2 = 2
sra a5, t3, t2          # x15 = -8 >> 2 = -2 (arithmetic shift preserves sign)

# --- Set Less Than (R-Type) ---
slt a6, t3, t1          # x16 = (-8 < 10) ? 1 : 0 = 1
sltu a7, t3, t1         # x17 = (unsigned -8 < 10) ? 1 : 0 = 0 (-8 is huge unsigned)

# --- Immediate Logical/Shift Operations (I-Type) ---
# Note: Re-using argument registers to save space
andi a0, t0, 15         # x10 = 12 & 15 = 12
ori  a1, t1, 5          # x11 = 10 | 5 = 15
xori a2, t0, -1         # x12 = 12 ^ -1 = -13 (bitwise NOT of 12)
slli a3, t1, 3          # x13 = 10 << 3 = 80
srli a4, t1, 1          # x14 = 10 >> 1 = 5
srai a5, t3, 1          # x15 = -8 >> 1 = -4

# --- Word-Level Operations (RV64I specific) ---
addiw a6, zero, 2047    # x16 = 2047 (Testing max positive 12-bit immediate)
addw  a7, t0, t1        # x17 = 12 + 10 = 22
subw  t4, t0, t1        # x29 = 12 - 10 = 2
sllw  t5, t1, t2        # x30 = 10 << 2 = 40 (computed at 32-bit, sign-extended)
srlw  t6, t1, t2        # x31 = 10 >> 2 = 2
sraw  t0, t3, t2        # x5  = -8 >> 2 = -2 (Overwrites our initial 12)

# --- Graceful Halt ---
done:
    beq zero, zero, done