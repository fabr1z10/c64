#include <iostream>
#include "d64parse.h"




void D64Parser::parse(const std::string &filename) {

    unsigned char data[0xf0000];
    const uint32_t STARTS[41] = {
            0,
            0x00000, 0x01500, 0x02a00, 0x03f00, 0x05400, 0x06900, 0x07e00, 0x09300,
            0x0a800, 0x0bd00, 0x0d200, 0x0e700, 0x0fc00, 0x11100, 0x12600, 0x13b00,
            0x15000, 0x16500, 0x17800, 0x18b00, 0x19e00, 0x1b100, 0x1c400, 0x1d700,
            0x1ea00, 0x1fc00, 0x20e00, 0x22000, 0x23200, 0x24400, 0x25600, 0x26700,
            0x27800, 0x28900, 0x29a00, 0x2ab00, 0x2bc00, 0x2cd00, 0x2de00, 0x2ef00
    };
    FILE* file = fopen(filename.c_str(), "rb");
    if (file == NULL) {
        std::cerr << "Can't find file: " << filename << "\n";
        exit(1);
    }
    int pos = 0;
    while (fread(&data[pos], 1, 1, file)) {
        pos++;
    }
    fclose(file);

    uint32_t base_dir = 0x16600;
    for (int i = 0; i < 0x1200; i += 0x20) {

    }

    std::cout << "read " << pos << " bytes.\n";
    for (int i = 0; i < 19; i++) {
        for (int j = 0; j < 256; ++j) {
            std::cout << std::hex << int(data[0x16500 + i*0x100 + j]) << ", ";
        }
        std::cout << "\n";
    }
//    uint32_t base_dir = 0x16600;
//
//
//    for (int i = 0; i < 0x1200; i += 0x20) {
//        std::cout << "next track: " << data[base_dir + i] << "\n";
//        std::cout << "next sec: " << data[base_dir + i + 1] << "\n";
//        std::cout << "file type: " << data[base_dir + i + 2] << "\n";
//    }

}