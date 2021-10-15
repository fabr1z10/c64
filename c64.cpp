#include "c64.h"
#include <iostream>


C64::C64() {

    m_opcodes[0x08] = [&] (uint16_t i) { this->php(i); };
    m_opcodes[0x18] = [&] (uint16_t i) { this->clc(i); };
    m_opcodes[0x28] = [&] (uint16_t i) { this->plp(i); };
    m_opcodes[0x38] = [&] (uint16_t i) { this->sec(i); };
    m_opcodes[0x48] = [&] (uint16_t i) { this->pha(i); };
    m_opcodes[0x58] = [&] (uint16_t i) { this->cli(i); };
    m_opcodes[0x68] = [&] (uint16_t i) { this->pla(i); };
    m_opcodes[0x78] = [&] (uint16_t i) { this->sei(i); };
    m_opcodes[0x88] = [&] (uint16_t i) { this->dey(i); };
    m_opcodes[0x98] = [&] (uint16_t i) { this->tya(i); };
    m_opcodes[0xA8] = [&] (uint16_t i) { this->tay(i); };
    m_opcodes[0xB8] = [&] (uint16_t i) { this->clv(i); };
    m_opcodes[0xC8] = [&] (uint16_t i) { this->iny(i); };
    m_opcodes[0xD8] = [&] (uint16_t i) { this->cld(i); };
    m_opcodes[0xE8] = [&] (uint16_t i) { this->inx(i); };
    m_opcodes[0xF8] = [&] (uint16_t i) { this->sed(i); };

    m_opcodes[0x09] = [&] (uint16_t i) { this->ora_imm(i); };
    m_opcodes[0x0D] = [&] (uint16_t i) { this->ora_abs(i); };
    m_opcodes[0x1D] = [&] (uint16_t i) { this->ora_abx(i); };
    m_opcodes[0x19] = [&] (uint16_t i) { this->ora_aby(i); };
    m_opcodes[0x05] = [&] (uint16_t i) { this->ora_zp(i); };
    m_opcodes[0x15] = [&] (uint16_t i) { this->ora_zpx(i); };
    m_opcodes[0x01] = [&] (uint16_t i) { this->ora_inx(i); };
    m_opcodes[0x11] = [&] (uint16_t i) { this->ora_iny(i); };

    m_opcodes[0x29] = [&] (uint16_t i) { this->and_imm(i); };
    m_opcodes[0x2D] = [&] (uint16_t i) { this->and_abs(i); };
    m_opcodes[0x3D] = [&] (uint16_t i) { this->and_abx(i); };
    m_opcodes[0x39] = [&] (uint16_t i) { this->and_aby(i); };
    m_opcodes[0x25] = [&] (uint16_t i) { this->and_zp(i); };
    m_opcodes[0x35] = [&] (uint16_t i) { this->and_zpx(i); };
    m_opcodes[0x21] = [&] (uint16_t i) { this->and_inx(i); };
    m_opcodes[0x31] = [&] (uint16_t i) { this->and_iny(i); };

    m_opcodes[0x49] = [&] (uint16_t i) { this->eor_imm(i); };
    m_opcodes[0x4D] = [&] (uint16_t i) { this->eor_abs(i); };
    m_opcodes[0x5D] = [&] (uint16_t i) { this->eor_abx(i); };
    m_opcodes[0x59] = [&] (uint16_t i) { this->eor_aby(i); };
    m_opcodes[0x45] = [&] (uint16_t i) { this->eor_zp(i); };
    m_opcodes[0x55] = [&] (uint16_t i) { this->eor_zpx(i); };
    m_opcodes[0x41] = [&] (uint16_t i) { this->eor_inx(i); };
    m_opcodes[0x51] = [&] (uint16_t i) { this->eor_iny(i); };

}

void C64::setNegativeFlag(uint8_t b) {
    if (b & 0x80) {       // negative
        m_sr |= 0x80;
    } else {
        m_sr &= 0x7F;
    }
}

void C64::setCarryFlag(bool value) {
    if (value) {
        m_sr |= 0x01u;
    } else {
        m_sr &= 0xFEu;
    }
}

void C64::setOverflowFlag(bool value) {
    if (value) {
        m_sr |= 0x40u;
    } else {
        m_sr &= 0xDFu;
    }
}

void C64::setZeroFlag(uint8_t b) {
    if (b == 0) {
        m_sr |= 0x02;
    } else {
        m_sr &= 0xFD;
    }
}

// indexed-indirect addressing X
uint8_t C64::addr_inx(uint8_t offset) {
    uint8_t zpa = m_x + offset;
    return m_memory[vecAsInt(zpa)];
}

// indexed-indirect addressing Y
uint8_t C64::addr_iny(uint8_t offset) {
    uint8_t zpa = m_y + offset;
    return m_memory[vecAsInt(zpa)];
}

uint16_t C64::vecAsInt(uint16_t address) {
    return m_memory[address] + m_memory[address+1] * 256;
}

Vector C64::getVec(uint16_t i) {
    return Vector{m_memory[i], m_memory[i+1]};
}

uint16_t Vector::toInt() const {
    return lo + hi * 256;
}

/* CLC (short for "CLear Carry") is the mnemonic for a machine language instruction which unconditionally clears the
 * carry flag.
*/
void C64::clc(uint16_t) {
    m_sr &= 0xFE;
}

/* SEC (short for "SEt Carry") is the mnemonic for a machine language instruction which unconditionally sets the carry
 * flag.
 */
void C64::sec(uint16_t) {
    m_sr |= 0x01;
}

/* PHP (short for "PusH Processor flags") is the mnemonic for a machine language instruction which stores the current
 * state of the processor status flags onto the stack, and adjusting the stack pointer to reflect the addition.
 */
void C64::php(uint16_t) {
    m_memory[m_sp] = m_sr;
    m_sp--;
    // TODO check stack overflow

}

/* PLP (short for "PulL Processor flags") is the mnemonic for a machine language instruction which retrieves a set
 * of status flags previously "pushed" onto the stack (usually by a PHP instruction) from the stack, and adjusting
 * the stack pointer to reflect the removal of a byte.
 */
void C64::plp(uint16_t) {
    m_sp++;
    m_sr = m_memory[m_sp];
    // TODO check stack underflow
}

/* PHA (short for "PusH Accumulator") is the mnemonic for a machine language instruction which stores a copy of the
 * current content of the accumulator onto the stack, and adjusting the stack pointer to reflect the addition.
 */
void C64::pha(uint16_t) {
    m_memory[m_sp] = m_a;
    m_sp--;
    // TODO check stack overflow
}

/* PLA (short for "PulL Accumulator") is the mnemonic for a machine language instruction which retrieves a byte from
 * the stack and stores it in the accumulator, and adjusts the stack pointer to reflect the removal of that byte.
 */
void C64::pla(uint16_t) {
    m_sp++;
    m_a = m_memory[m_sp];

    // The negative status flag is set if the retrieved byte is negative, i.e. has its most significant bit set.
    setNegativeFlag(m_a);

    // The zero flag is set if the retrieved byte is zero, or cleared if it is non-zero.
    setZeroFlag(m_a);
}

/* CLI (short for "CLear Interrupt flag") is the mnemonic for a machine language instruction which clears the interrupt
 * flag, so that the CPU will respond to IRQ interrupt events. To disable the response to IRQs, use the complementary
 * instruction SEI.
 */
void C64::cli(uint16_t) {
    m_sr &= 0xFB;
}

/* SEI (short for "SEt Interrupt flag") is the mnemonic for a machine language instruction which sets the interrupt
 * flag, thereby preventing the CPU from responding to IRQ interrupt events. To re-enable the response to IRQs, use the
 * complementary instruction CLI.
 */
void C64::sei(uint16_t) {
    m_sr |= 0x04;
    m_pc++;
}

/* DEY (short for "DEcrease Y") is the mnemonic for a machine language instruction which decrements the numerical value
 * of Y index register, by one, and "wraps over" if the value goes below the numerical limits of a byte.
 */
void C64::dey(uint16_t) {
    m_y--;
    setNegativeFlag(m_y);
    setZeroFlag(m_y);
}

/* TYA (short for "Transfer Y to Accumulator") is the mnemonic for a machine language instruction which transfers
 * ("copies") the contents of the Y index register into the accumulator.
 */
void C64::tya(uint16_t) {
    m_a = m_y;

    // The negative status flag is set if the byte transferred is negative, i.e. has it's most significant bit set.
    setNegativeFlag(m_a);

    // The zero flag is set if the result is zero, or cleared if it is non-zero.
    setZeroFlag(m_a);
}

/* TAY (short for "Transfer Accumulator to Y") is the mnemonic for a machine language instruction which transfers
 * ("copies") the contents of the accumulator into the Y index register.
 */
void C64::tay(uint16_t) {
   m_y = m_a;

   // The negative status flag is set if the byte transferred is negative, i.e. has it's most significant bit set.
   setNegativeFlag(m_y);

   // The zero flag is set if the byte transferred is zero, or cleared if it is non-zero.
   setZeroFlag(m_y);

}

/* CLV (short for "CLear oVerflow") is the mnemonic for a machine language instruction which clears the Overflow flag.
 */
void C64::clv(uint16_t) {
    m_sr &= 0xBF;
}

/* INY (short for "INcrease Y") is the mnemonic for a machine language instruction which increases the numerical value
 * held in the Y index register by one, and "wraps over" when the numerical limits of a byte are exceeded.
 */
void C64::iny(uint16_t) {
    m_y++;

    // The negative status flag is set if the result is negative, i.e. has its most significant bit set.
    setNegativeFlag(m_y);

    // The zero flag is set if the result is zero, or cleared if it is non-zero.
    setZeroFlag(m_y);
}

/* CLD (short for "CLear Decimal flag") is the mnemonic for a machine language instruction which clears the decimal
 * flag. It complements the SED (set decimal flag) instruction.
 */
void C64::cld(uint16_t) {
   m_sr &= 0xF7;
}

/* INX (short for "INcrease X") is the mnemonic for a machine language instruction which increases the numerical
 * value held in the X index register by one, and "wraps over" when the numerical limits of a byte are exceeded.
 */
void C64::inx(uint16_t) {
    m_x++;

    // The negative status flag is set if the result is negative, i.e. has its most significant bit set.
    setNegativeFlag(m_x);

    // The zero flag is set if the result is zero, or cleared if it is non-zero.
    setZeroFlag(m_x);
}

/* SED (short for "SEt Decimal flag") is the mnemonic for a machine language instruction which sets the decimal flag.
 * It complements the CLD (clear decimal flag) instruction.
 */
void C64::sed(uint16_t) {
    m_sr |= 0x08;
}

void C64::ora_imm(uint16_t i) {
    m_a |= m_memory[i+1];
    setNegativeFlag(m_a);
    setZeroFlag(m_a);
}

void C64::ora_abs(uint16_t i) {
    auto addr = getVec(i+1).toInt();
    m_a |= m_memory[addr];
    setNegativeFlag(m_a);
    setZeroFlag(m_a);
}

void C64::ora_abx(uint16_t i) {
    auto addr = getVec(i+1).toInt() + m_x;
    m_a |= m_memory[addr];
    setNegativeFlag(m_a);
    setZeroFlag(m_a);
}

void C64::ora_aby(uint16_t i) {
    auto addr = getVec(i+1).toInt() + m_y;
    m_a |= m_memory[addr];
    setNegativeFlag(m_a);
    setZeroFlag(m_a);
}

void C64::ora_zp(uint16_t i) {
    m_a |= m_memory[m_memory[i+1]];
    setNegativeFlag(m_a);
    setZeroFlag(m_a);
}

void C64::ora_zpx(uint16_t i) {
    auto addr = m_memory[i+1] + m_x;
    m_a |= m_memory[addr];
    setNegativeFlag(m_a);
    setZeroFlag(m_a);
}

void C64::ora_inx(uint16_t i) {
    m_a |= addr_inx(m_memory[i+1]);
    setNegativeFlag(m_a);
    setZeroFlag(m_a);
}

void C64::ora_iny(uint16_t i) {
    m_a |= addr_iny(m_memory[i+1]);
    setNegativeFlag(m_a);
    setZeroFlag(m_a);
}

void C64::and_imm(uint16_t i) {
    m_a &= m_memory[i+1];
    setNegativeFlag(m_a);
    setZeroFlag(m_a);
}

void C64::and_abs(uint16_t i) {
    auto addr = getVec(i+1).toInt();
    m_a &= m_memory[addr];
    setNegativeFlag(m_a);
    setZeroFlag(m_a);
}

void C64::and_abx(uint16_t i) {
    auto addr = getVec(i+1).toInt() + m_x;
    m_a &= m_memory[addr];
    setNegativeFlag(m_a);
    setZeroFlag(m_a);
}

void C64::and_aby(uint16_t i) {
    auto addr = getVec(i+1).toInt() + m_y;
    m_a &= m_memory[addr];
    setNegativeFlag(m_a);
    setZeroFlag(m_a);
}

void C64::and_zp(uint16_t i) {
    m_a &= m_memory[m_memory[i+1]];
    setNegativeFlag(m_a);
    setZeroFlag(m_a);
}

void C64::and_zpx(uint16_t i) {
    m_a &= m_memory[m_memory[i+1] + m_x];
    setNegativeFlag(m_a);
    setZeroFlag(m_a);
}

void C64::and_inx(uint16_t i) {
    m_a &= addr_inx(m_memory[i+1]);
    setNegativeFlag(m_a);
    setZeroFlag(m_a);
}

void C64::and_iny(uint16_t i) {
    m_a &= addr_iny(m_memory[i+1]);
    setNegativeFlag(m_a);
    setZeroFlag(m_a);
}

void C64::eor_imm(uint16_t i) {
    m_a ^= m_memory[i+1];
    setNegativeFlag(m_a);
    setZeroFlag(m_a);
}

void C64::eor_abs(uint16_t i) {
    auto addr = getVec(i+1).toInt();
    m_a ^= m_memory[addr];
    setNegativeFlag(m_a);
    setZeroFlag(m_a);
}

void C64::eor_abx(uint16_t i) {
    auto addr = getVec(i+1).toInt() + m_x;
    m_a ^= m_memory[addr];
    setNegativeFlag(m_a);
    setZeroFlag(m_a);
}

void C64::eor_aby(uint16_t i) {
    auto addr = getVec(i+1).toInt() + m_y;
    m_a ^= m_memory[addr];
    setNegativeFlag(m_a);
    setZeroFlag(m_a);
}

void C64::eor_zp(uint16_t i) {
    m_a ^= m_memory[m_memory[i+1]];
    setNegativeFlag(m_a);
    setZeroFlag(m_a);
}

void C64::eor_zpx(uint16_t i) {
    m_a ^= m_memory[m_memory[i+1] + m_x];
    setNegativeFlag(m_a);
    setZeroFlag(m_a);
}

void C64::eor_inx(uint16_t i) {
    m_a ^= addr_inx(m_memory[i+1]);
    setNegativeFlag(m_a);
    setZeroFlag(m_a);
}

void C64::eor_iny(uint16_t i) {
    m_a ^= addr_iny(m_memory[i+1]);
    setNegativeFlag(m_a);
    setZeroFlag(m_a);
}

void C64::adc_imm(uint16_t i) {
    uint8_t carry = m_sr & 0x01;
    uint16_t tmp = m_a + carry + m_memory[i+1];
    // the lsb goes to a, the msb holds info about carry
    uint8_t tmp2 = tmp & 0x00FFu;
    bool overflow = (((m_a ^ m_memory[i+1]) & 0x80u) == 0) && (((m_a ^ tmp2) & 0x80u) == 1);
    m_a = tmp2;
    uint8_t nc = tmp & 0x0100u;
    setCarryFlag(nc != 0);
    setOverflowFlag(overflow);
    setZeroFlag(m_a);
    setNegativeFlag(m_a);
    // TODO check overflow


}

void C64::exec(uint16_t i) {
    // set the program counter
    m_pc = i;
    bool continueLoop = true;
    int co = 0;
    while (co < 2) {
        // fetch the opcode
        uint8_t opcode = m_memory[m_pc];
        std::cout << "opcode : " << int(opcode) << " ";
        auto f = m_opcodes[opcode];
        if (f) {
            uint16_t old_pc = m_pc;
            f(m_pc);
            for (auto i = old_pc+1; i < m_pc; ++i) {
                std::cout << int(m_memory[i]) << " ";
            }
            std::cout << std::endl;
        } else {
            std::cout << " !! unknown opcode !!\n";
        }
        co++;
    };
}

void C64::load_prg(const std::vector<uint8_t> & data) {
    // address is contained in the first 2 bytes
    uint16_t addr = data[0] + data[1] * 256;
    std::cout << "storing program @ " << addr << std::endl;
    for (size_t i = 2; i < data.size(); ++i) {
        m_memory[addr++] = data[i];
    }
}