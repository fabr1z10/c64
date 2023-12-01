#pragma once

#include <cstdint>


class VICII {
public:
	VICII();
	uint8_t* getPtr(int);
private:
	uint8_t* _reg;
};