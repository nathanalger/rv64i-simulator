addi x1, x0, 0
addi x2, x0, 3

loop:
addi x1, x1, 1
beq x1, x2, end
jal x0, loop

end:
add x3, x1, x2