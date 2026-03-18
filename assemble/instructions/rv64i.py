from registry import register
from formats import *
from parse_reg import parse_reg

def assemble_sw(parts, labels, current_address):
    if len(parts) != 3:
        raise ValueError(f"Invalid sw format at address {current_address}")

    rs2 = parse_reg(parts[1])

    # Safely parse offset(rs1)
    offset_part = parts[2]
    if "(" not in offset_part or not offset_part.endswith(")"):
        raise ValueError(f"Invalid memory operand '{offset_part}' at address {current_address}")

    imm_str, reg_str = offset_part.split("(")
    imm = int(imm_str.strip())
    rs1 = parse_reg(reg_str.replace(")", ""))

    return encode_s(imm, rs2, rs1, 0b010, 0b0100011)

def assemble_sub(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid sub format at address {current_address}")

    rd = parse_reg(parts[1])
    rs1 = parse_reg(parts[2])
    rs2 = parse_reg(parts[3])

    return encode_r(0b0100000, rs2, rs1, 0b000, rd, 0b0110011)

def assemble_lw(parts, labels, current_address):
    if len(parts) != 3:
        raise ValueError(f"Invalid lw format at address {current_address}")

    rd = parse_reg(parts[1])

    # Safely parse offset(rs1)
    offset_part = parts[2]
    if "(" not in offset_part or not offset_part.endswith(")"):
        raise ValueError(f"Invalid memory operand '{offset_part}' at address {current_address}")

    imm_str, reg_str = offset_part.split("(")
    imm = int(imm_str.strip())
    rs1 = parse_reg(reg_str.replace(")", ""))

    return encode_i(imm, rs1, 0b010, rd, 0b0000011)

def assemble_jal(parts, labels, current_address):
    if len(parts) != 3:
        raise ValueError(f"Invalid jal format at address {current_address}")

    rd = parse_reg(parts[1])
    target = parts[2]

    # Resolve label or immediate
    if target in labels:
        imm = labels[target] - current_address
    else:
        imm = int(target)

    # Note: Range and alignment checks are safely handled in formats.py
    return encode_j(imm, rd, 0b1101111)

def assemble_beq(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid beq format at address {current_address}")

    rs1 = parse_reg(parts[1])
    rs2 = parse_reg(parts[2])
    target = parts[3]

    if target in labels:
        offset = labels[target] - current_address
    else:
        offset = int(target)

    # Note: Range and alignment checks are safely handled in formats.py
    return encode_b(offset, rs2, rs1, 0b000, 0b1100011)

def assemble_addi(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid addi format at address {current_address}")

    rd = parse_reg(parts[1])
    rs1 = parse_reg(parts[2])
    imm = int(parts[3])

    return encode_i(imm, rs1, 0b000, rd, 0b0010011)

def assemble_add(parts, labels, current_address):
    # Safety check: 'add rd, rs1, rs2' should have exactly 4 parts
    if len(parts) != 4:
        raise ValueError(f"Invalid add format at address {current_address}. Expected: add rd, rs1, rs2")

    # Use parse_reg instead of string replacement
    rd = parse_reg(parts[1])
    rs1 = parse_reg(parts[2])
    rs2 = parse_reg(parts[3])

    # funct7=0x00, funct3=0x0, opcode=0x33
    return encode_r(0b0000000, rs2, rs1, 0b000, rd, 0b0110011)

register("sw", assemble_sw)
register("sub", assemble_sub)
register("lw", assemble_lw)
register("jal", assemble_jal)
register("beq", assemble_beq)
register("addi", assemble_addi)
register("add", assemble_add)