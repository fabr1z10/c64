#pragma once

#include <vector>
#include <string>

class C64 {

public:
    C64 ();
    std::vector<uint8_t> m_memory;
    void step();                    // executes next instruction
    uint16_t m_pc;                  // program counter
    uint8_t m_a;                    // accumulator
    uint8_t m_x, m_y;               // x and y reg
    uint16_t vecAsInt(uint16_t);
    uint8_t inx(uint8_t);
    uint8_t iny(uint8_t);
};