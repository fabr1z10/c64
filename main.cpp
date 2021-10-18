#include <iostream>
#include "d64parse.h"
#include "c64.h"

int main() {


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