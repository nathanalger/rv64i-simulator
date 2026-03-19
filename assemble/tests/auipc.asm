auipc x5, 1         # PC = 0.   Offset = 0x1000.  Result: x5  = 0x1000
auipc x6, 2         # PC = 4.   Offset = 0x2000.  Result: x6  = 0x2004
auipc x7, 0         # PC = 8.   Offset = 0.       Result: x7  = 0x0008
auipc x28, 0xFFFFF  # PC = 12.  Offset = -4096.   Result: x28 = -4084 (0xFFFFFFFFFFFFF00C)
beq x0, x0, 0       # PC = 16.  Infinite loop/halt to end the test