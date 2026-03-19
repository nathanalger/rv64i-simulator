from registry import register
from formats import *
from parse_reg import parse_reg

def resolve_offset(target, labels, current_address):
    if target in labels:
        return labels[target] - current_address
    return int(target)

def assemble_bne(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid bne format at address {current_address}")

    rs1 = parse_reg(parts[1])
    rs2 = parse_reg(parts[2])
    offset = resolve_offset(parts[3], labels, current_address)

    return encode_b(offset, rs2, rs1, 0b001, 0b1100011)


def assemble_blt(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid blt format at address {current_address}")

    rs1 = parse_reg(parts[1])
    rs2 = parse_reg(parts[2])
    offset = resolve_offset(parts[3], labels, current_address)

    return encode_b(offset, rs2, rs1, 0b100, 0b1100011)


def assemble_bge(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid bge format at address {current_address}")

    rs1 = parse_reg(parts[1])
    rs2 = parse_reg(parts[2])
    offset = resolve_offset(parts[3], labels, current_address)

    return encode_b(offset, rs2, rs1, 0b101, 0b1100011)


def assemble_bltu(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid bltu format at address {current_address}")

    rs1 = parse_reg(parts[1])
    rs2 = parse_reg(parts[2])
    offset = resolve_offset(parts[3], labels, current_address)

    return encode_b(offset, rs2, rs1, 0b110, 0b1100011)


def assemble_bgeu(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid bgeu format at address {current_address}")

    rs1 = parse_reg(parts[1])
    rs2 = parse_reg(parts[2])
    offset = resolve_offset(parts[3], labels, current_address)

    return encode_b(offset, rs2, rs1, 0b111, 0b1100011)

def assemble_jalr(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid jalr format at address {current_address}")

    rd = parse_reg(parts[1])
    rs1 = parse_reg(parts[2])
    imm = int(parts[3], 0)

    return encode_i(imm, rs1, 0b000, rd, 0b1100111)

register("bne", assemble_bne)
register("blt", assemble_blt)
register("bge", assemble_bge)
register("bltu", assemble_bltu)
register("bgeu", assemble_bgeu)

register("jalr", assemble_jalr)