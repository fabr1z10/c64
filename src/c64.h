#pragma once

#include <vector>
#include <string>
#include <array>
#include <functional>
#include <memory>
#include "vicii.h"
#include "settings.h"

enum Flag {
    CARRY = 0,
    ZERO = 1,
    INTERRUPT_DISABLE = 2,
    DECIMAL_MODE = 3,
    BREAK = 4,
    OVERFLOW = 6,
    NEGATIVE = 7
};

enum AddressMode {
    IMPLIED,
    ACCUMULATOR,
    IMMEDIATE,
    ABSOLUTE,
    ABSOLUTE_X,
    ABSOLUTE_Y,
    ZEROPAGE,
    ZEROPAGE_INDEXED,
    RELATIVE,
    INDEXED_INDIRECT,
    INDIRECT_INDEXED
};





inline int const COLOR_COUNT = 16;


inline const int NUMBER_OF_LINES[2] = {312, 263};
inline const int CYCLES_PER_LINE[2] = {63, 65};

inline const double CLOCK_FREQUENCY[2] = {0.9852486e6, 1.0227273e6};









struct Vector {
    uint8_t lo;
    uint8_t hi;
    uint16_t toInt() const;
};

class C64;
class Shader;
class MainShader;

struct OpcodeInfo {

    std::string text;
    AddressMode addressMode;
    uint8_t bytes;
    uint8_t cycles;
    void (C64::*methodPtrOne)();


};

class C64 {
public:
    explicit C64(Mode mode);
    ~C64();
    uint8_t readByte(uint16_t address) const;
    uint16_t readVec(uint16_t address) const;
    void writeVec(uint16_t address, uint16_t value);
    void test();
    void run();
private:

	std::string disassemble(const OpcodeInfo& opcode, uint16_t address);
	std::unique_ptr<VICII> _vic;
	long _clockCycle;
	uint8_t* _kernal;
	uint8_t* _basic;
	uint8_t* _charRom;
	uint8_t* _ram;

	std::unique_ptr<Shader> _blitShader;
	std::unique_ptr<MainShader> _mainShader;
	void initializeGL();
	void initOpcodes();
    uint8_t getBit(uint8_t value, uint8_t bit);
    void setBit(uint8_t& ref, uint8_t value, uint8_t bit);
    // stack operations
    void push(uint8_t);
    void pushVec(uint16_t);
    uint8_t pop();
    uint16_t popVec();
    void setNegFlag(const uint8_t&);
    void setZeroFlag(const uint8_t&);
    void setCarryFlag(const uint16_t&);
    void branch(bool);
    void brk();
    void php();
    void clc();
    void bpl();
    void jsr();
    void plp();
    void bmi();
    void sec();
    void rti();
    void pha();
    void jmp_abs();
    void jmp_ind();
    void bvc();
    void cli();
    void rts();
    void pla();
    void bvs();
    void sei();
    void txs();
    void cld();



    template<int length, uint8_t (C64::*addr)()>
    void ora() {
        auto value = (*this.*addr)();
        _a |= value;
        // The negative status flag is set if the result is negative, i.e. has its most significant bit set.
        setNegFlag(_a);
        // The zero flag is set if the result is zero, or cleared if it is non-zero.
        setZeroFlag(_a);
        _pc += length;
    }

    template<int length, uint8_t (C64::*addr)()>
    void _and() {
        auto value = (*this.*addr)();
        _a &= value;
        // The negative status flag is set if the result is negative, i.e. has its most significant bit set.
        setNegFlag(_a);
        // The zero flag is set if the result is zero, or cleared if it is non-zero.
        setZeroFlag(_a);
        _pc += length;
    }

    template<int length, uint8_t (C64::*addr)()>
    void eor() {
        auto value = (*this.*addr)();
        _a ^= value;
        // The negative status flag is set if the result is negative, i.e. has its most significant bit set.
        setNegFlag(_a);
        // The zero flag is set if the result is zero, or cleared if it is non-zero.
        setZeroFlag(_a);
        _pc += length;
    }

    // ADd with Carry
    template<int length, uint8_t (C64::*addr)()>
    void adc() {
        auto value = (*this.*addr)();
        auto msb = _a & 0x80;
        bool sameSign = (msb == (value & 0x80));
        uint16_t result = _a + value;
        uint8_t carry = ((result & 0xFF00) != 0 ? 1 : 0);
        setBit(_status, carry, 0);
        setZeroFlag(_a);
        auto msb_after = _a & 0x80;
        // The overflow flag is thus set when the most significant bit (here considered the sign bit) is changed by
        // adding two numbers with the same sign (or subtracting two numbers with opposite signs).
        setBit(_status, (sameSign && (msb != msb_after) ? 1 : 0), Flag::OVERFLOW);

    }


    template<int length, uint8_t(C64::*addr)()>
    void bit() {
        auto value = (*this.*addr)();
        if ((value & 0x80) == 0) {
            _status &= 0x7F;
        } else {
            _status |= 0x80;
        }
        if ((value & 0x40) == 0) {
            _status &= 0xBF;
        } else {
            _status |= 0x40;
        }
        auto result = _a | value;
        setZeroFlag(result);
        _pc += length;
    }

    // STore Accumulator
    template<int length, uint8_t& (C64::*addr)()>
    void sta() {
        auto& value = (*this.*addr)();
        value = _a;
        _pc += length;
    }

    template<int length, uint8_t& (C64::*addr)()>
    void stx() {
        auto& value = (*this.*addr)();
        value = _x;
        _pc += length;
    }

    template<int length, uint8_t& (C64::*addr)()>
    void sty() {
        auto& value = (*this.*addr)();
        value = _y;
        _pc += length;
    }

    // arithmetic shift left
    template<int length, uint8_t& (C64::*addr)()>
    void asl() {
        auto& value = (*this.*addr)();
        bool set_carry = (value & 0x80) != 0;
        if (set_carry) {
            _status |= 0x01;
        } else {
            _status &= 0xFE;
        }
        value <<= 1;
        setNegFlag(value);
        setZeroFlag(value);
    }

    // rotate left
    template<int length, uint8_t& (C64::*addr)()>
    void rol() {
        auto& value = (*this.*addr)();
        // get current carry
        uint8_t carry = getBit(_status, 0);
        uint8_t msb = getBit(value, 7);
        setBit(_status, msb, 0);
        value <<= 1;
        setBit(value, carry, 0);
        setNegFlag(value);
        setZeroFlag(value);
    }

    // rotate right
    template<int length, uint8_t& (C64::*addr)()>
    void ror() {
        auto& value = (*this.*addr)();
        // get current carry
        uint8_t carry = getBit(_status, 0);
        uint8_t lsb = getBit(value, 0);
        setBit(_status, lsb, Flag::CARRY);
        value >>= 1;
        setBit(value, carry, 7);
        setNegFlag(value);
        setZeroFlag(value);
    }


    // logic shift right
    template<int length, uint8_t& (C64::*addr)()>
    void lsr() {
        auto& value = (*this.*addr)();
        uint8_t carry = getBit(value, 0);
        setBit(_status, carry, 0);
        value >>= 1;
        setNegFlag(value);
        setZeroFlag(value);
    }

    template<int length, uint8_t (C64::*addr)()>
    void ldx() {
    	auto value = (*this.*addr)();
    	_x = value;
    	_pc += length;
    }

	template<int length, uint8_t (C64::*addr)()>
	void lda() {
		auto value = (*this.*addr)();
		_a = value;
		_pc += length;

		// The negative status flag is set if the result is negative, i.e. has it's most significant bit set.
		setNegFlag(_a);

		// The zero flag is set if the result is zero, or cleared if it is non-zero.
		setZeroFlag(_a);
	}

	template<int length, uint8_t (C64::*addr)()>
	void cmp() {
		auto operand = (*this.*addr)();
		uint16_t result = _a - operand;
		uint8_t lb = (uint8_t) result & 0xFF;
		setCarryFlag(result);
		setNegFlag(lb);
		setZeroFlag(lb);
		_pc += length;

	}


    uint8_t& getRefAcc() {
        return _a;
    }
    uint8_t& getRefAbs();
    uint8_t& getRefAbx();
    uint8_t& getRefZP();
    uint8_t& getRefZPx();
    uint8_t& getRefInx();
    uint8_t& getRefIny();
    uint8_t getOperandImm();
    uint8_t getOperandAbs();
    uint8_t getOperandAbx();
    uint8_t getOperandAby();
    uint8_t getOperandInx();
    uint8_t getOperandIny();
    uint8_t getOperandZP();
    uint8_t getOperandZPx();




    uint8_t* getPtr(uint16_t address) const;
    uint16_t strToVec(const std::string&);
    void readFile(const std::string& filename, uint8_t* ptr);


    uint16_t _pc;               // program counter (16 bits)
    uint8_t _a, _x, _y;
    uint8_t _sp;

    // Bit  Flag
    // 0    Carry
    // 1    Zero
    // 2    Interrupt disable
    // 3    Decimal mode
    // 4    Break
    // 5    Unused
    // 6    Overflow
    // 7    Negative
    uint8_t _status;
    std::vector<OpcodeInfo> _opcodes;
    Mode _mode;



};