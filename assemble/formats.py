def encode_r(funct7, rs2, rs1, funct3, rd, opcode):
    validate_reg(rs1, rs2, rd)
    
    return (
        (funct7 << 25) |
        (rs2 << 20) |
        (rs1 << 15) |
        (funct3 << 12) |
        (rd << 7) |
        opcode
    )


def encode_i(imm, rs1, funct3, rd, opcode):
    validate_reg(rs1, rd)

    # 12-bit signed immediate [-2048 to 2047]
    if not (-2048 <= imm <= 2047):
        raise ValueError(f"Immediate {imm} out of range! (Must be between -2048 and 2047)")
    
    return (
        ((imm & 0xFFF) << 20) |
        (rs1 << 15) |
        (funct3 << 12) |
        (rd << 7) |
        opcode
    )


def encode_s(imm, rs2, rs1, funct3, opcode):
    validate_reg(rs1, rs2)

    # 12-bit signed immediate [-2048 to 2047]
    if not (-2048 <= imm <= 2047):
        raise ValueError(f"Immediate {imm} out of range! (Must be between -2048 and 2047)")

    imm_11_5 = (imm >> 5) & 0x7F
    imm_4_0 = imm & 0x1F

    return (
        (imm_11_5 << 25) |
        (rs2 << 20) |
        (rs1 << 15) |
        (funct3 << 12) |
        (imm_4_0 << 7) |
        opcode
    )


def encode_b(imm, rs2, rs1, funct3, opcode):
    validate_reg(rs1, rs2)
    # 1. Range Check: 13-bit signed immediate [-4096 to 4094]
    if not (-4096 <= imm <= 4094):
        raise ValueError(f"Branch offset {imm} out of range! (Must be between -4096 and 4094)")

    # 2. Alignment Check: RISC-V instructions must be at least 2-byte aligned
    if imm % 2 != 0:
        raise ValueError(f"Branch offset {imm} is not 2-byte aligned!")
    
    # Branch immediates are weirdly split
    imm_12   = (imm >> 12) & 0x1
    imm_10_5 = (imm >> 5) & 0x3F
    imm_4_1  = (imm >> 1) & 0xF
    imm_11   = (imm >> 11) & 0x1

    return (
        (imm_12 << 31) |
        (imm_10_5 << 25) |
        (rs2 << 20) |
        (rs1 << 15) |
        (funct3 << 12) |
        (imm_4_1 << 8) |
        (imm_11 << 7) |
        opcode
    )


def encode_u(imm, rd, opcode):
    validate_reg(rd)
    return (
        (imm & 0xFFFFF000) |
        (rd << 7) |
        opcode
    )


def encode_j(imm, rd, opcode):
    validate_reg(rd)
    # J-type immediate is 21 bits signed [-1048576 to 1048574]
    if not (-1048576 <= imm <= 1048574):
        raise ValueError(f"JAL displacement {imm} out of range!")
    
    # Convert to unsigned 21-bit two's complement
    imm &= 0x1FFFFF  # keep lowest 21 bits

    imm_20    = (imm >> 20) & 0x1
    imm_10_1  = (imm >> 1) & 0x3FF
    imm_11    = (imm >> 11) & 0x1
    imm_19_12 = (imm >> 12) & 0xFF

    print("[DEBUG encode_j] imm =", imm)
    print("[DEBUG encode_j] imm_20   =", imm_20)
    print("[DEBUG encode_j] imm_10_1 =", imm_10_1)
    print("[DEBUG encode_j] imm_11   =", imm_11)
    print("[DEBUG encode_j] imm_19_12 =", imm_19_12)
    print("[DEBUG encode_j] rd =", rd, "opcode =", bin(opcode))

    instruction = (
        (imm_20 << 31) |
        (imm_19_12 << 12) |
        (imm_11 << 20) |
        (imm_10_1 << 21) |
        (rd << 7) |
        opcode
    )

    print("[DEBUG encode_j] encoded instruction =", hex(instruction))
    return instruction

def validate_reg(*regs):
    for reg in regs:
        if not (0 <= reg <= 31):
            raise ValueError(f"Register x{reg} is out of bounds! Must be 0-31.")