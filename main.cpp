#include <iostream>
#include "d64parse.h"
#include "c64.h"

void test1(C64& computer) {
    computer.poke(0x0400, 0xFF);
    computer.poke(0x800, {0xee, 0x00, 0x04});
    computer.setProgramCounter(0x800);
    computer.step();
    std::cout << (int) computer.peek(0x0400) << "\n";
}

int main() {
//    C64 computer;
//    test1(computer);
//
//    return 0;

    uint8_t a = 1;
    uint8_t b = 2;
    uint8_t aa = a-b;
    std::cout << (int) aa << "\n";
    exit(1);
    D64Parser p;
    p.parse("/home/fabrizio/Downloads/Barbarian - The Ultimate Warrior (1987)(Palace Software)[cr F4CG].d64");
    auto c = p.getData(0);
    std::cout << "---\n";
    for (int i = 0; i < c.size(); i++) {
        std::cout << std::hex << int(c[i]) << ", ";
        if (i % 8 == 7) {
            std::cout << "\n";
        }
    }
    C64 computer;

    // load opcodes

    computer.load_prg(c);
    computer.exec(0x0811);
}

