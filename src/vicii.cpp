#include "vicii.h"

VICII::VICII() {

}




uint8_t * VICII::getPtr(int value) {
	if (value < 47) {
		return &_reg[value];
	}
	return nullptr;
}