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

struct OpcodeInfo {
    std::string text;
    std::string addressMode;
    uint8_t bytes;
};

class C64 {

public:
    C64 ();
    std::array<uint8_t, 65536> m_memory;
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
    uint16_t sign_extend(uint8_t);
    std::array<std::function<void(int)>, 256> m_opcodes;
    std::array<OpcodeInfo, 256> m_opcodeInfos;
    void setNegativeFlag(uint8_t);
    void setZeroFlag(uint8_t);
    void setCarryFlag(bool);
    void setOverflowFlag(bool);
    void load_prg(const std::vector<uint8_t>&);
    void exec(uint16_t);
    void step(uint16_t);
    bool isDecimalMode() const;
    uint8_t peek(uint16_t);
    void poke(uint16_t address, uint8_t value);
    void poke(uint16_t address, const std::vector<uint8_t>& data);
    void setProgramCounter(uint16_t);
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

    /* ORA (short for "Logical OR on Accumulator") is the mnemonic for a machine language instruction which performs a
     * bit-wise boolean "or" between each of the eight bits in the accumulator and their corresponding bits in the
     * memory address specified. The eight resulting bits form a byte, which is stored in the accumulator.
     */
    void ora_imm(uint16_t);
    void ora_abs(uint16_t);
    void ora_abx(uint16_t);
    void ora_aby(uint16_t);
    void ora_zp(uint16_t);
    void ora_zpx(uint16_t);
    void ora_inx(uint16_t);
    void ora_iny(uint16_t);

    /* AND (short for "Logic AND") is the mnemonic for a machine language instruction which performs a bit-wise boolean
     * "and" between the eight bits in the operand and the eight bits of the accumulator. Each resulting bit is a one
     * (1) only if the corresponding bits of the operand and the accumulator are also one (1). If either corresponding
     * bit is zero (0) then the resulting bit will be zero (0). The result is stored in the accumulator. In this way,
     * the AND operation masks the value in the accumulator with the value in the operand.
     */
    void and_imm(uint16_t);
    void and_abs(uint16_t);
    void and_abx(uint16_t);
    void and_aby(uint16_t);
    void and_zp(uint16_t);
    void and_zpx(uint16_t);
    void and_inx(uint16_t);
    void and_iny(uint16_t);

    /* EOR (short for "Exclusive OR") is the mnemonic for a machine language instruction which performs a bit-wise
     * boolean "Exclusive-or" between each of the eight bits in the accumulator and their corresponding bits in the
     * memory address specified. The eight resulting bits form a byte, which is stored in the accumulator.
     */
    void eor_imm(uint16_t);
    void eor_abs(uint16_t);
    void eor_abx(uint16_t);
    void eor_aby(uint16_t);
    void eor_zp(uint16_t);
    void eor_zpx(uint16_t);
    void eor_inx(uint16_t);
    void eor_iny(uint16_t);

    /* ADC (short for "ADd with Carry") is the mnemonic for a machine language instruction which adds the byte held in
     * the accumulator with that held in the memory address specified: The state of the carry flag before the addition
     * takes place, is taken as the incoming carry in the addition. After the addition, the carry flag will hold the
     * outgoing carry.
     */
    void adc_imm(uint16_t);


    /* LDA (short for "LoaD Accumulator") is the mnemonic for a machine language instruction which retrieves a copy
     * from the specified RAM or I/O address, and stores it in the accumulator. The content of the memory location is
     * not affected by the operation.
     */
    void lda_imm(uint16_t);
    void lda_aby(uint16_t);

    /* LDY (short for "LoaD Y") is the mnemonic for a machine language instruction which retrieves a copy from the
     * specified RAM or I/O address, and stores it in the Y index register. The content of the memory location is not
     * affected by the operation.
     */
    void ldy_imm(uint16_t);


    /* STA (short for "STore Accumulator") is the mnemonic for a machine language instruction which stores a copy of
     * the byte held in the accumulator at the RAM or I/O address specified. The contents of the accumulator itself
     * remains unchanged through the operation.
     */
    void sta_aby(uint16_t);
    void sta_zp(uint16_t);


    /* STY (short for "STore Y") is the mnemonic for a machine language instruction which stores a copy of the byte
     * held in the Y index register at the RAM or I/O address specified. The contents of the Y index register itself
     * remains unchanged through the operation.
     */
    void sty_zp(uint16_t);


    /* BNE (short for "Branch if Not Equal") is the mnemonic for a machine language instruction which branches, or
     * "jumps", to the address specified if, and only if the zero flag is clear. If the zero flag is set when the CPU
     * encounters a BNE instruction, the CPU will continue at the instruction following the BNE rather than taking the
     * jump.
     * BNE only supports the Relative addressing mode, as shown in the table at right. In the assembler formats listed,
     * nn is a one-byte (8-bit) relative address. The relative address is treated as a signed byte; that is, it shifts
     * program execution to a location within a number of bytes ranging from -128 to 127, relative to the address of the
     * instruction *** following *** the branch instruction.
     */
    void bne(uint16_t);

    /* JMP (short for "JuMP") is the mnemonic for a machine language instruction which unconditionally transfers program
     * execution to the specified address. To those familiar with BASIC programming; this is the machine language
     * equivalent to GOTO.
     */
    void jmp(uint16_t);

    /* INC (short for "INCrease") is the mnemonic for a machine language instruction which increases the numerical value
     * of the contents of the address specified by one, and "wraps over" when the numerical limits of a byte are
     * exceeded.
     */
    void inc_abs(uint16_t);

    /* CMP (short for "CoMPare") is the mnemonic for a machine language instruction which compares the contents of the
     * accumulator against that of the specified operand by subtracting operand from accumulator value, and setting the
     * negative and carry flags according to the result. Unlike SBC, the result of the subtraction is discarded rather
     * than stored back into the accumulator, which is thus unaffected by the CMP operation.
     */
};