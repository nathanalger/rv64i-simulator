# --- SETUP ---
addi sp, zero, 128       # ADDI: Initialize stack pointer to memory address 128
addi a0, zero, 3         # ADDI: Set loop counter to 3
addi t0, zero, 10        # ADDI: Starting value to manipulate

# --- START EXECUTION ---
jal ra, memory_test      # JAL: Jump to our test block, saving return address in 'ra'

done:
beq zero, zero, done     # BEQ: Infinite loop to safely "halt" the program

# --- TEST FUNCTION ---
memory_test:
# 1. Arithmetic Test
addi t1, zero, 5         # ADDI: Load 5 into t1
add t0, t0, t1           # ADD: t0 = t0 + 5 (15, then 20, then 25...)

# 2. Save / Load Test
sw t0, 0(sp)             # SW: Store the value of t0 into memory at the address in sp
lw t2, 0(sp)             # LW: Load the memory value back into t2

# 3. Verification Test
sub t3, t0, t2           # SUB: t3 = t0 - t2. If SW and LW work perfectly, t3 will ALWAYS be 0!

# 4. Loop Pointer Updates
addi t4, zero, 1         # ADDI: Load 1 into t4
sub a0, a0, t4           # SUB: Decrement our loop counter (a0 = a0 - 1)
addi sp, sp, 4           # ADDI: Advance the stack pointer by 4 bytes (1 word)

# 5. Branching Test
beq a0, zero, exit_test  # BEQ: If our loop counter hits 0, jump to exit
jal zero, memory_test    # JAL: Otherwise, unconditionally jump back to the top of the loop

exit_test:
jal zero, done           # JAL: Jump back to the main program loop