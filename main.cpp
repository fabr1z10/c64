#include <iostream>
#include "d64parse.h"

int main() {
    D64Parser p;
    p.parse("/home/fabrizio/Downloads/Barbarian - The Ultimate Warrior (1987)(Palace Software)[cr F4CG].d64");
    auto c = p.getData(0);
    for (int i = 0; i < c.size(); i++) {
        std::cout << int(c[i]) << ", ";
        if (i % 8 == 0) {
            std::cout << "\n";
        }
    }
}