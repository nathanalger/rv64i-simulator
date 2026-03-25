#include "Processor.h"

/**
 * Full disclosure, this was written entirely by AI. This is an unholy mess that I do not care
 * to write by hand.
 */
uint32_t Processor::decompress(uint16_t c)
{
   uint8_t op = c & 0x3; // Quadrant
   uint8_t funct3 = (c >> 13) & 0x7;

   switch (op)
   {
   case 0: // Quadrant 0
      switch (funct3)
      {
      case 0:
      { // C.ADDI4SPN -> addi rd', x2, imm
         if (c == 0)
            return 0xFFFFFFFF; // All zeros is an illegal instruction

         uint32_t rd = ((c >> 2) & 0x7) + 8;
         uint32_t imm = (((c >> 7) & 0xF) << 6) |
                        (((c >> 11) & 0x3) << 4) |
                        (((c >> 5) & 0x1) << 3) |
                        (((c >> 6) & 0x1) << 2);

         // Return: ADDI rd, x2, imm
         return (imm << 20) | (2 << 15) | (0 << 12) | (rd << 7) | 0x13;
      }
      case 2:
      { // C.LW -> lw rd', offset(rs1')
         uint32_t rd = ((c >> 2) & 0x7) + 8;
         uint32_t rs1 = ((c >> 7) & 0x7) + 8;
         uint32_t imm = (((c >> 6) & 0x1) << 2) | (((c >> 10) & 0x7) << 3) | (((c >> 5) & 0x1) << 6);
         return (imm << 20) | (rs1 << 15) | (2 << 12) | (rd << 7) | 0x03;
      }
      case 3:
      { // C.LD -> ld rd', offset(rs1')
         uint32_t rd = ((c >> 2) & 0x7) + 8;
         uint32_t rs1 = ((c >> 7) & 0x7) + 8;
         uint32_t imm = (((c >> 10) & 0x7) << 3) | (((c >> 5) & 0x3) << 6);
         return (imm << 20) | (rs1 << 15) | (3 << 12) | (rd << 7) | 0x03;
      }
      case 6:
      { // C.SW -> sw rs2', offset(rs1')
         uint32_t rs2 = ((c >> 2) & 0x7) + 8;
         uint32_t rs1 = ((c >> 7) & 0x7) + 8;
         uint32_t imm = (((c >> 6) & 0x1) << 2) | (((c >> 10) & 0x7) << 3) | (((c >> 5) & 0x1) << 6);
         return (((imm >> 5) & 0x7F) << 25) | (rs2 << 20) | (rs1 << 15) | (2 << 12) | ((imm & 0x1F) << 7) | 0x23;
      }
      case 7:
      { // C.SD -> sd rs2', offset(rs1')
         uint32_t rs2 = ((c >> 2) & 0x7) + 8;
         uint32_t rs1 = ((c >> 7) & 0x7) + 8;
         uint32_t imm = (((c >> 10) & 0x7) << 3) | (((c >> 5) & 0x3) << 6);
         return (((imm >> 5) & 0x7F) << 25) | (rs2 << 20) | (rs1 << 15) | (3 << 12) | ((imm & 0x1F) << 7) | 0x23;
      }
      }
      break;

   case 1: // Quadrant 1
      switch (funct3)
      {
      case 0:
      { // C.ADDI -> addi rd, rd, imm
         uint32_t rd = (c >> 7) & 0x1F;
         if (rd == 0)
            return 0x00000013; // C.NOP
         int32_t imm = ((c >> 2) & 0x1F) | ((c & 0x1000) ? 0xFFFFFFE0 : 0);
         return ((imm & 0xFFF) << 20) | (rd << 15) | (0 << 12) | (rd << 7) | 0x13;
      }
      case 1:
      { // C.ADDIW -> addiw rd, rd, imm
         uint32_t rd = (c >> 7) & 0x1F;
         int32_t imm = ((c >> 2) & 0x1F) | ((c & 0x1000) ? 0xFFFFFFE0 : 0);
         return ((imm & 0xFFF) << 20) | (rd << 15) | (0 << 12) | (rd << 7) | 0x1B;
      }
      case 2:
      { // C.LI -> addi rd, x0, imm (This is what failed!)
         uint32_t rd = (c >> 7) & 0x1F;
         int32_t imm = ((c >> 2) & 0x1F) | ((c & 0x1000) ? 0xFFFFFFE0 : 0);
         return ((imm & 0xFFF) << 20) | (0 << 15) | (0 << 12) | (rd << 7) | 0x13;
      }
      case 3:
      { // C.LUI / C.ADDI16SP
         uint32_t rd = (c >> 7) & 0x1F;
         if (rd == 2)
         { // C.ADDI16SP -> addi x2, x2, imm
            int32_t imm = (((c >> 6) & 0x1) << 4) | (((c >> 2) & 0x1) << 5) | (((c >> 5) & 0x1) << 6) | (((c >> 3) & 0x3) << 7) | ((c & 0x1000) ? 0xFFFFFE00 : 0);
            return ((imm & 0xFFF) << 20) | (2 << 15) | (0 << 12) | (2 << 7) | 0x13;
         }
         else
         { // C.LUI -> lui rd, imm
            int32_t imm = (((c >> 2) & 0x1F) << 12) | ((c & 0x1000) ? 0xFFFE0000 : 0);
            return (imm & 0xFFFFF000) | (rd << 7) | 0x37;
         }
      }
      case 4:
      { // ALU Operations (SRLI, SRAI, ANDI, SUB, XOR, OR, AND, etc.)
         uint32_t funct2 = (c >> 10) & 0x3;
         uint32_t rd = ((c >> 7) & 0x7) + 8;
         if (funct2 == 0)
         { // C.SRLI -> srli rd, rd, shamt
            uint32_t shamt = (((c >> 2) & 0x1F) | (((c >> 12) & 0x1) << 5));
            return (shamt << 20) | (rd << 15) | (5 << 12) | (rd << 7) | 0x13;
         }
         else if (funct2 == 1)
         { // C.SRAI -> srai rd, rd, shamt
            uint32_t shamt = (((c >> 2) & 0x1F) | (((c >> 12) & 0x1) << 5));
            return (0x20 << 25) | (shamt << 20) | (rd << 15) | (5 << 12) | (rd << 7) | 0x13;
         }
         else if (funct2 == 2)
         { // C.ANDI -> andi rd, rd, imm
            int32_t imm = ((c >> 2) & 0x1F) | ((c & 0x1000) ? 0xFFFFFFE0 : 0);
            return ((imm & 0xFFF) << 20) | (rd << 15) | (7 << 12) | (rd << 7) | 0x13;
         }
         else if (funct2 == 3)
         {
            uint32_t rs2 = ((c >> 2) & 0x7) + 8;
            uint32_t funct1 = (c >> 5) & 0x3;
            if ((c >> 12) & 1)
            { // ADDW / SUBW
               if (funct1 == 0)
                  return (0x20 << 25) | (rs2 << 20) | (rd << 15) | (0 << 12) | (rd << 7) | 0x3B; // SUBW
               if (funct1 == 1)
                  return (0x00 << 25) | (rs2 << 20) | (rd << 15) | (0 << 12) | (rd << 7) | 0x3B; // ADDW
            }
            else
            { // SUB / XOR / OR / AND
               if (funct1 == 0)
                  return (0x20 << 25) | (rs2 << 20) | (rd << 15) | (0 << 12) | (rd << 7) | 0x33; // SUB
               if (funct1 == 1)
                  return (0x00 << 25) | (rs2 << 20) | (rd << 15) | (4 << 12) | (rd << 7) | 0x33; // XOR
               if (funct1 == 2)
                  return (0x00 << 25) | (rs2 << 20) | (rd << 15) | (6 << 12) | (rd << 7) | 0x33; // OR
               if (funct1 == 3)
                  return (0x00 << 25) | (rs2 << 20) | (rd << 15) | (7 << 12) | (rd << 7) | 0x33; // AND
            }
         }
      }
      break;
      case 5:
      { // C.J -> jal x0, offset
         int32_t imm = (((c >> 3) & 0x7) << 1) | (((c >> 11) & 0x1) << 4) | (((c >> 2) & 0x1) << 5) | (((c >> 7) & 0x1) << 6) | (((c >> 6) & 0x1) << 7) | (((c >> 9) & 0x3) << 8) | (((c >> 8) & 0x1) << 10) | (((c >> 12) & 0x1) << 11);
         if (c & 0x1000)
            imm |= 0xFFFFF000;
         uint32_t imm20_10_1_11_19_12 = (((imm >> 20) & 0x1) << 31) | (((imm >> 1) & 0x3FF) << 21) | (((imm >> 11) & 0x1) << 20) | (((imm >> 12) & 0xFF) << 12);
         return imm20_10_1_11_19_12 | (0 << 7) | 0x6F;
      }
      case 6: // C.BEQZ -> beq rs1', x0, offset
      case 7:
      { // C.BNEZ -> bne rs1', x0, offset
         uint32_t rs1 = ((c >> 7) & 0x7) + 8;
         int32_t imm = (((c >> 3) & 0x3) << 1) | (((c >> 10) & 0x3) << 3) | (((c >> 2) & 0x1) << 5) | (((c >> 5) & 0x3) << 6) | (((c >> 12) & 0x1) << 8);
         if (c & 0x1000)
            imm |= 0xFFFFFE00;
         uint32_t funct3_b = (funct3 == 6) ? 0 : 1;
         return (((imm >> 12) & 0x1) << 31) | (((imm >> 5) & 0x3F) << 25) | (0 << 20) | (rs1 << 15) | (funct3_b << 12) | (((imm >> 1) & 0xF) << 8) | (((imm >> 11) & 0x1) << 7) | 0x63;
      }
      }
      break;

   case 2: // Quadrant 2
      switch (funct3)
      {
      case 0:
      { // C.SLLI -> slli rd, rd, shamt
         uint32_t rd = (c >> 7) & 0x1F;
         uint32_t shamt = ((c >> 2) & 0x1F) | (((c >> 12) & 0x1) << 5);
         return (shamt << 20) | (rd << 15) | (1 << 12) | (rd << 7) | 0x13;
      }
      case 1:
      { // C.FLDSP -> fld rd, offset(x2)
         uint32_t rd = (c >> 7) & 0x1F;

         // Offset calculation (matches C.LDSP)
         uint32_t imm = (((c >> 5) & 0x3) << 3) | (((c >> 12) & 0x1) << 5) | (((c >> 2) & 0x7) << 6);

         // Map to: FLD rd, offset(x2)
         // I-Type format: opcode = 0x07, funct3 = 3, rs1 = 2 (sp)
         return (imm << 20) | (2 << 15) | (3 << 12) | (rd << 7) | 0x07;
      }
      case 2:
      { // C.LWSP -> lw rd, offset(x2)
         uint32_t rd = (c >> 7) & 0x1F;
         uint32_t imm = (((c >> 4) & 0x7) << 2) | (((c >> 12) & 0x1) << 5) | (((c >> 2) & 0x3) << 6);
         return (imm << 20) | (2 << 15) | (2 << 12) | (rd << 7) | 0x03;
      }
      case 3:
      { // C.LDSP -> ld rd, offset(x2)
         uint32_t rd = (c >> 7) & 0x1F;
         uint32_t imm = (((c >> 5) & 0x3) << 3) | (((c >> 12) & 0x1) << 5) | (((c >> 2) & 0x7) << 6);
         return (imm << 20) | (2 << 15) | (3 << 12) | (rd << 7) | 0x03;
      }
      case 4:
      {
         uint32_t rd = (c >> 7) & 0x1F;
         uint32_t rs2 = (c >> 2) & 0x1F;
         if ((c & 0x1000) == 0)
         {
            if (rs2 == 0)
               return (0 << 20) | (rd << 15) | (0 << 12) | (0 << 7) | 0x67; // C.JR -> jalr x0, rd, 0
            else
               return (0 << 25) | (rs2 << 20) | (0 << 15) | (0 << 12) | (rd << 7) | 0x33; // C.MV -> add rd, x0, rs2
         }
         else
         {
            if (rs2 == 0)
            {
               if (rd == 0)
                  return 0x00100073; // C.EBREAK
               else
                  return (0 << 20) | (rd << 15) | (0 << 12) | (1 << 7) | 0x67; // C.JALR -> jalr x1, rd, 0
            }
            else
               return (0 << 25) | (rs2 << 20) | (rd << 15) | (0 << 12) | (rd << 7) | 0x33; // C.ADD -> add rd, rd, rs2
         }
      }
      case 5: // C.FSDSP
      {
         uint32_t rs2 = (c >> 2) & 0x1F;

         // Offset for C.FSDSP: imm[5:3] are at c[12:10], imm[8:6] are at c[9:7]
         uint32_t offset = (((c >> 10) & 0x7) << 3) | (((c >> 7) & 0x7) << 6);

         // Convert to S-Type Immediate format
         uint32_t imm11_5 = (offset >> 5) & 0x7F;
         uint32_t imm4_0 = offset & 0x1F;

         // Map to: FSD rs2, offset(x2)
         // opcode = 0x27, funct3 = 3, rs1 = 2 (sp)
         return (imm11_5 << 25) | (rs2 << 20) | (2 << 15) | (3 << 12) | (imm4_0 << 7) | 0x27;
      }
      case 6:
      { // C.SWSP -> sw rs2, offset(x2)
         uint32_t rs2 = (c >> 2) & 0x1F;
         uint32_t imm = (((c >> 9) & 0xF) << 2) | (((c >> 7) & 0x3) << 6);
         return (((imm >> 5) & 0x7F) << 25) | (rs2 << 20) | (2 << 15) | (2 << 12) | ((imm & 0x1F) << 7) | 0x23;
      }
      case 7:
      { // C.SDSP -> sd rs2, offset(x2)
         uint32_t rs2 = (c >> 2) & 0x1F;
         uint32_t imm = (((c >> 10) & 0x7) << 3) | (((c >> 7) & 0x7) << 6);
         return (((imm >> 5) & 0x7F) << 25) | (rs2 << 20) | (2 << 15) | (3 << 12) | ((imm & 0x1F) << 7) | 0x23;
      }
      }
      break;
   }

   // Fallback triggers a proper Trap 2 (Illegal Instruction)
   return 0xFFFFFFFF;
}