#pragma once

#include <vector>
#include <string>
#include <array>
#include <functional>

struct Vector {
    uint8_t lo;
    uint8_t hi;
    uint16_t toInt() const;
};

class C64 {

public:
    C64 ();
    std::vector<uint8_t> m_memory;
    void step();                    // executes next instruction
    uint16_t m_pc;                  // program counter
    uint8_t m_a;                    // accumulator
    uint8_t m_x, m_y;               // x and y reg

    /*
     * m_sr represents the 65xx status register.
     * Table 1 - Flags of the MOS Technology 65xx Processor Status Register
     * Bit 	Flag 	          Abbreviation 	Purpose
     * 0 	Carry Flag 	            C 	    Indicates when a bit of the result is to be carried to or borrowed from another byte. Also used for rotate and shift operations.
       1 	Zero Flag 	            Z 	    A one indicates that the result of an operation is equal to zero.
       2 	Interrupt Disable Flag 	I 	    If set IRQ will be prevented (masked), except non-maskable interrupts (NMI).
       3 	Decimal Mode Flag 	    D 	    If set arithmetic operations are calculated in decimal mode (otherwise usually in binary mode).
       4 	Break Command Flag 	    B 	    Indicates that interrupt request has been triggered by an BRK opcode (not an IRQ).
       5 	Unused 	                - 	    Cannot be changed, usually 1.
       6 	Overflow Flag 	        V 	    Indicates that a result of an signed arithmetic operation exceeds the signed value range (-128 to 127).
       7 	Negative Flag 	        N 	    A value of 1 indicates that the result is negative (bit 7 is set, for a two's complement representation).
    */
    uint8_t m_sr;
    uint8_t m_sp;                   // the stack pointer
    uint16_t vecAsInt(uint16_t);
    Vector getVec(uint16_t);
    uint8_t addr_inx(uint8_t);
    uint8_t addr_iny(uint8_t);

    std::array<std::function<void(int)>, 256> m_opcodes;
    void setNegativeFlag(uint8_t);
    void setZeroFlag(uint8_t);
private:
    void clc(uint16_t);
    void sec(uint16_t);
    void php(uint16_t);
    void plp(uint16_t);
    void pha(uint16_t);
    void pla(uint16_t);
    void cli(uint16_t);
    void sei(uint16_t);
    void dey(uint16_t);
    void tya(uint16_t);
    void tay(uint16_t);
    void clv(uint16_t);
    void iny(uint16_t);
    void cld(uint16_t);
    void inx(uint16_t);
    void sed(uint16_t);
};