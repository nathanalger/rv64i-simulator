from registry import register
from formats import encode_s
from parse_reg import parse_reg
from parse_mem import parse_mem_operand

def assemble_sb(parts, labels, current_address):
    if len(parts) != 3:
        raise ValueError(f"Invalid sb format at address {current_address}")

    rs2 = parse_reg(parts[1])
    imm, rs1 = parse_mem_operand(parts[2], current_address)

    return encode_s(imm, rs2, rs1, 0b000, 0b0100011)


def assemble_sh(parts, labels, current_address):
    if len(parts) != 3:
        raise ValueError(f"Invalid sh format at address {current_address}")

    rs2 = parse_reg(parts[1])
    imm, rs1 = parse_mem_operand(parts[2], current_address)

    return encode_s(imm, rs2, rs1, 0b001, 0b0100011)


def assemble_sw(parts, labels, current_address):
    if len(parts) != 3:
        raise ValueError(f"Invalid sw format at address {current_address}")

    rs2 = parse_reg(parts[1])
    imm, rs1 = parse_mem_operand(parts[2], current_address)

    return encode_s(imm, rs2, rs1, 0b010, 0b0100011)


def assemble_sd(parts, labels, current_address):
    if len(parts) != 3:
        raise ValueError(f"Invalid sd format at address {current_address}")

    rs2 = parse_reg(parts[1])
    imm, rs1 = parse_mem_operand(parts[2], current_address)

    return encode_s(imm, rs2, rs1, 0b011, 0b0100011)


register("sb", assemble_sb)
register("sh", assemble_sh)
register("sw", assemble_sw)
register("sd", assemble_sd)