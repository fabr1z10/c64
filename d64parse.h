#pragma once

#include <string>
#include <vector>

struct Entry {
    uint8_t next_track;
    uint8_t next_sector;
    std::string file_type;
    uint8_t start_track;
    uint8_t start_sector;
    std::string pet_name;
    uint16_t sector_size;
    uint32_t adress_start;
    uint32_t adress_end;
};

class D64Parser {

public:
    D64Parser();
    void parse(const std::string& file);
    std::vector<uint8_t> getData(uint8_t row_id);
private:
    std::string FILE_TYPE[0x100];
    const uint32_t STARTS[41] = {
            0,
            0x00000, 0x01500, 0x02a00, 0x03f00, 0x05400, 0x06900, 0x07e00, 0x09300,
            0x0a800, 0x0bd00, 0x0d200, 0x0e700, 0x0fc00, 0x11100, 0x12600, 0x13b00,
            0x15000, 0x16500, 0x17800, 0x18b00, 0x19e00, 0x1b100, 0x1c400, 0x1d700,
            0x1ea00, 0x1fc00, 0x20e00, 0x22000, 0x23200, 0x24400, 0x25600, 0x26700,
            0x27800, 0x28900, 0x29a00, 0x2ab00, 0x2bc00, 0x2cd00, 0x2de00, 0x2ef00
    };
    std::vector<Entry> entries;
    unsigned char data[0xf0000];
};