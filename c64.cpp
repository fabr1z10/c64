#include "c64.h"

C64::C64() {


}

// indexed-indirect addressing X
uint8_t C64::inx(uint8_t offset) {
    uint8_t zpa = m_x + offset;
    return m_memory[vecAsInt(zpa)];
}

// indexed-indirect addressing Y
uint8_t C64::iny(uint8_t offset) {
    uint8_t zpa = m_y + offset;
    return m_memory[vecAsInt(zpa)];
}

uint16_t C64::vecAsInt(uint16_t address) {
    return m_memory[address] + m_memory[address+1] * 256;
}