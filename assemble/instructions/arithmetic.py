from registry import register
from formats import encode_r, encode_i
from parse_reg import parse_reg

# ----------------------
# R-type Instructions
# ----------------------
def assemble_and(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid and format at address {current_address}")
    rd = parse_reg(parts[1])
    rs1 = parse_reg(parts[2])
    rs2 = parse_reg(parts[3])
    return encode_r(0b0000000, rs2, rs1, 0b111, rd, 0b0110011)

def assemble_or(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid or format at address {current_address}")
    rd = parse_reg(parts[1])
    rs1 = parse_reg(parts[2])
    rs2 = parse_reg(parts[3])
    return encode_r(0b0000000, rs2, rs1, 0b110, rd, 0b0110011)

def assemble_xor(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid xor format at address {current_address}")
    rd = parse_reg(parts[1])
    rs1 = parse_reg(parts[2])
    rs2 = parse_reg(parts[3])
    return encode_r(0b0000000, rs2, rs1, 0b100, rd, 0b0110011)

def assemble_sll(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid sll format at address {current_address}")
    rd = parse_reg(parts[1])
    rs1 = parse_reg(parts[2])
    rs2 = parse_reg(parts[3])
    return encode_r(0b0000000, rs2, rs1, 0b001, rd, 0b0110011)

def assemble_srl(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid srl format at address {current_address}")
    rd = parse_reg(parts[1])
    rs1 = parse_reg(parts[2])
    rs2 = parse_reg(parts[3])
    return encode_r(0b0000000, rs2, rs1, 0b101, rd, 0b0110011)

def assemble_sra(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid sra format at address {current_address}")
    rd = parse_reg(parts[1])
    rs1 = parse_reg(parts[2])
    rs2 = parse_reg(parts[3])
    return encode_r(0b0100000, rs2, rs1, 0b101, rd, 0b0110011)

def assemble_slt(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid slt format at address {current_address}")
    rd = parse_reg(parts[1])
    rs1 = parse_reg(parts[2])
    rs2 = parse_reg(parts[3])
    return encode_r(0b0000000, rs2, rs1, 0b010, rd, 0b0110011)

def assemble_sltu(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid sltu format at address {current_address}")
    rd = parse_reg(parts[1])
    rs1 = parse_reg(parts[2])
    rs2 = parse_reg(parts[3])
    return encode_r(0b0000000, rs2, rs1, 0b011, rd, 0b0110011)

# ----------------------
# I-type Instructions
# ----------------------
def assemble_andi(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid andi format at address {current_address}")
    rd = parse_reg(parts[1])
    rs1 = parse_reg(parts[2])
    imm = int(parts[3], 0)
    return encode_i(imm, rs1, 0b111, rd, 0b0010011)

def assemble_ori(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid ori format at address {current_address}")
    rd = parse_reg(parts[1])
    rs1 = parse_reg(parts[2])
    imm = int(parts[3], 0)
    return encode_i(imm, rs1, 0b110, rd, 0b0010011)

def assemble_xori(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid xori format at address {current_address}")
    rd = parse_reg(parts[1])
    rs1 = parse_reg(parts[2])
    imm = int(parts[3], 0)
    return encode_i(imm, rs1, 0b100, rd, 0b0010011)

def assemble_slli(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid slli format at address {current_address}")
    rd = parse_reg(parts[1])
    rs1 = parse_reg(parts[2])
    imm = int(parts[3], 0)
    return encode_i(imm, rs1, 0b001, rd, 0b0010011)

def assemble_srli(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid srli format at address {current_address}")
    rd = parse_reg(parts[1])
    rs1 = parse_reg(parts[2])
    imm = int(parts[3], 0)
    return encode_i(imm, rs1, 0b101, rd, 0b0010011)

def assemble_srai(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid srai format at address {current_address}")
    rd = parse_reg(parts[1])
    rs1 = parse_reg(parts[2])
    imm = int(parts[3], 0)
    
    # 0x400 is 1024, which sets exactly bit 10 of the immediate (bit 30 of the instruction)
    return encode_i(imm | 0x400, rs1, 0b101, rd, 0b0010011)

# ----------------------
# RV64W 32-bit Instructions
# ----------------------
def assemble_addw(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid addw format at address {current_address}")
    rd = parse_reg(parts[1])
    rs1 = parse_reg(parts[2])
    rs2 = parse_reg(parts[3])
    return encode_r(0b0000000, rs2, rs1, 0b000, rd, 0b0111011)

def assemble_subw(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid subw format at address {current_address}")
    rd = parse_reg(parts[1])
    rs1 = parse_reg(parts[2])
    rs2 = parse_reg(parts[3])
    return encode_r(0b0100000, rs2, rs1, 0b000, rd, 0b0111011)

def assemble_addiw(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid addiw format at address {current_address}")
    rd = parse_reg(parts[1])
    rs1 = parse_reg(parts[2])
    imm = int(parts[3], 0)
    return encode_i(imm, rs1, 0b000, rd, 0b0011011)

def assemble_sllw(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid sllw format at address {current_address}")
    rd = parse_reg(parts[1])
    rs1 = parse_reg(parts[2])
    rs2 = parse_reg(parts[3])
    return encode_r(0b0000000, rs2, rs1, 0b001, rd, 0b0111011)

def assemble_srlw(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid srlw format at address {current_address}")
    rd = parse_reg(parts[1])
    rs1 = parse_reg(parts[2])
    rs2 = parse_reg(parts[3])
    return encode_r(0b0000000, rs2, rs1, 0b101, rd, 0b0111011)

def assemble_sraw(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid sraw format at address {current_address}")
    rd = parse_reg(parts[1])
    rs1 = parse_reg(parts[2])
    rs2 = parse_reg(parts[3])
    return encode_r(0b0100000, rs2, rs1, 0b101, rd, 0b0111011)

# ----------------------
# Register all instructions
# ----------------------
register("and", assemble_and)
register("or", assemble_or)
register("xor", assemble_xor)
register("sll", assemble_sll)
register("srl", assemble_srl)
register("sra", assemble_sra)
register("slt", assemble_slt)
register("sltu", assemble_sltu)

register("andi", assemble_andi)
register("ori", assemble_ori)
register("xori", assemble_xori)
register("slli", assemble_slli)
register("srli", assemble_srli)
register("srai", assemble_srai)

register("addw", assemble_addw)
register("subw", assemble_subw)
register("addiw", assemble_addiw)
register("sllw", assemble_sllw)
register("srlw", assemble_srlw)
register("sraw", assemble_sraw)