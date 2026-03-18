from registry import register
from formats import encode_i
from parse_reg import parse_reg

def assemble(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid addi format at address {current_address}")

    rd = parse_reg(parts[1])
    rs1 = parse_reg(parts[2])
    imm = int(parts[3])

    return encode_i(imm, rs1, 0b000, rd, 0b0010011)

register("addi", assemble)