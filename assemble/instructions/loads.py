from registry import register
from formats import encode_i
from parse_reg import parse_reg
from parse_mem import parse_mem_operand

def assemble_lb(parts, labels, current_address):
    if len(parts) != 3:
        raise ValueError(f"Invalid lb format at address {current_address}")

    rd = parse_reg(parts[1])
    imm, rs1 = parse_mem_operand(parts[2], current_address)

    return encode_i(imm, rs1, 0b000, rd, 0b0000011)

def assemble_lh(parts, labels, current_address):
    if len(parts) != 3:
        raise ValueError(f"Invalid lh format at address {current_address}")

    rd = parse_reg(parts[1])
    imm, rs1 = parse_mem_operand(parts[2], current_address)

    return encode_i(imm, rs1, 0b001, rd, 0b0000011)

def assemble_lw(parts, labels, current_address):
    if len(parts) != 3:
        raise ValueError(f"Invalid lw format at address {current_address}")

    rd = parse_reg(parts[1])
    imm, rs1 = parse_mem_operand(parts[2], current_address)

    return encode_i(imm, rs1, 0b010, rd, 0b0000011)

def assemble_ld(parts, labels, current_address):
    if len(parts) != 3:
        raise ValueError(f"Invalid ld format at address {current_address}")

    rd = parse_reg(parts[1])
    imm, rs1 = parse_mem_operand(parts[2], current_address)

    return encode_i(imm, rs1, 0b011, rd, 0b0000011)

def assemble_lbu(parts, labels, current_address):
    if len(parts) != 3:
        raise ValueError(f"Invalid lbu format at address {current_address}")

    rd = parse_reg(parts[1])
    imm, rs1 = parse_mem_operand(parts[2], current_address)

    return encode_i(imm, rs1, 0b100, rd, 0b0000011)

def assemble_lhu(parts, labels, current_address):
    if len(parts) != 3:
        raise ValueError(f"Invalid lhu format at address {current_address}")

    rd = parse_reg(parts[1])
    imm, rs1 = parse_mem_operand(parts[2], current_address)

    return encode_i(imm, rs1, 0b101, rd, 0b0000011)

def assemble_lwu(parts, labels, current_address):
    if len(parts) != 3:
        raise ValueError(f"Invalid lwu format at address {current_address}")

    rd = parse_reg(parts[1])
    imm, rs1 = parse_mem_operand(parts[2], current_address)

    return encode_i(imm, rs1, 0b110, rd, 0b0000011)

register("lb", assemble_lb)
register("lh", assemble_lh)
register("lw", assemble_lw)
register("ld", assemble_ld)
register("lbu", assemble_lbu)
register("lhu", assemble_lhu)
register("lwu", assemble_lwu)