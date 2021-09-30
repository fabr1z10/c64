#pragma once

#include <string>

struct Entry {
    uint8_t next_track;
    uint8_t next_sector;
};

class D64Parser {

public:
    void parse(const std::string& file);
};