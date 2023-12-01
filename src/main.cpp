#include <iostream>
#include <memory>
#include "c64.h"





int window_width;
int window_height;

//
//void test1(C64& computer) {
//    computer.poke(0x0400, 0xFF);
//    computer.poke(0x800, {0xee, 0x00, 0x04});
//    computer.setProgramCounter(0x800);
//    computer.step();
//    std::cout << (int) computer.peek(0x0400) << "\n";
//}







int main() {
	C64 computer(Mode::PAL);
	computer.run();



    //glfwSetKeyCallback(window, key_callback);







}

