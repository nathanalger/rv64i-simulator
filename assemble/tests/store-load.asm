# Base address
addi t0, x0, 100        # t0 = base address

# Values (safe immediates)
addi t1, x0, 17         # 0x11   -> for SB
addi t2, x0, 546        # 0x222  -> for SH
addi t3, x0, 819        # 0x333  -> for SW
addi t4, x0, 1024       # 0x400  -> for SD

# ---- STORES ----

sb t1, 0(t0)            # [100]     = 0x11
sh t2, 2(t0)            # [102-103] = 0x0222
sw t3, 4(t0)            # [104-107] = 0x00000333
sd t4, 8(t0)            # [108-115] = 0x0000000000000400

lb  t5, 0(t0)
lh  t6, 2(t0)
lw  t1, 4(t0)
ld  t2, 8(t0)

# Infinite loop to stop execution
loop:
beq x0, x0, loop