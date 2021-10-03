#include <iostream>
#include "d64parse.h"


D64Parser::D64Parser() {

    FILE_TYPE[0x80] = "DEL";
    FILE_TYPE[0x81] = "SEQ";
    FILE_TYPE[0x82] = "PRG";
    FILE_TYPE[0x83] = "USR";
    FILE_TYPE[0x84] = "REL";
}

std::vector<uint8_t> D64Parser::getData(uint8_t row_id) {
    const auto& entry = entries.at(row_id);
    std::vector<uint8_t> ret(entry.sector_size * 0x100);
    int c = 0;
    uint32_t next_track = entry.start_track;
    uint32_t next_sector = entry.start_sector;
    while (c < entry.sector_size) {
        uint32_t a_adr = STARTS[next_track] + next_sector * 256;
        for (int i = 0; i < 254; i++) {
            ret[c*254 + i] = data[a_adr + i + 2];
        }
        next_track = data[a_adr];
        next_sector = data[a_adr + 1];
        c++;
    }
    return ret;
}

void D64Parser::parse(const std::string &filename) {

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
        Entry entry;
        entry.next_track = data[base_dir + i];
        entry.next_sector = data[base_dir + i + 1];
        entry.file_type = FILE_TYPE[data[base_dir + i + 2]];
        entry.start_track = data[base_dir + i + 3];
        entry.start_sector = data[base_dir + i + 4];
        entry.sector_size = data[base_dir + i + 0x1e] + (data[base_dir + i + 0x1f] * 256);
        for (int j = 5; j < 0x15; j++) {
            entry.pet_name += data[base_dir + i + j];
        }
        std::cout << "entry: " << int(entry.next_track) << "/" << int(entry.next_sector) << " " << entry.file_type << " " <<
            int(entry.start_track) << "/" << int(entry.start_sector) << " :" << entry.pet_name << " " << entry.sector_size << "\n";
        entry.adress_start = STARTS[entry.start_track] + entry.start_sector * 256;
        entry.adress_end = entry.adress_start + entry.sector_size * 256;
        entries.push_back(entry);
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