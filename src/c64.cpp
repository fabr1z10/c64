#include "c64.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <chrono>
#include <thread>
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <GLFW/glfw3.h>
#include "shader.h"
#include "shaders.h"


using namespace std;
using namespace std::chrono;



GLFWwindow* window;


void WindowResizeCallback(GLFWwindow* win, int width, int height) {
	// notify cameras
	if (height == 0) height = 1;
	settings::window_width = width;
	settings::window_height = height;
	//glViewport(0, 0, width, height);
}

C64::C64(Mode mode) : _mode(mode), _clockCycle(0) {
	settings::mode = mode;
	if (mode == Mode::PAL) {
		settings::width = settings::PAL_SCREEN_WIDTH;
		settings::height = settings::PAL_SCREEN_HEIGHT;
		settings::visible_width = settings::PAL_VISIBLE_WIDTH;
		settings::visible_height = settings::PAL_VISIBLE_HEIGHT;
	} else {
		settings::width = settings::NTSC_SCREEN_WIDTH;
		settings::height = settings::NTSC_SCREEN_HEIGHT;
	}

	initializeGL();

    _kernal = new uint8_t[8192];
    _basic = new uint8_t[8192];
    _charRom = new uint8_t[4096];
    _ram = new uint8_t[65536];

    initOpcodes();
    readFile ("/home/fabrizio/c64/rom/kernal", &_kernal[0]);
    readFile ("/home/fabrizio/c64/rom/basic", &_basic[0]);
    readFile ("/home/fabrizio/c64/rom/chargen", &_charRom[0]);
    memset(_ram, 0x00, 65536);

	// initialize RAM
	//
	_ram[0x0000] = 0x2F;				// processor port data direction register
    _ram[0x0001] = 0x37;				// processor port
    writeVec(0x0003, 0xB1AA);		// execution address of routine converting floating point to integer.
    writeVec(0x0005, 0xB391);		// execution address of routine converting integer to floating point.
	_ram[0x0016] = 0x19;				// Pointer to next expression in string stack
	writeVec(0x002B, 0x0801);		// Pointer to beginning of BASIC area.
	writeVec(0x0037, 0xA000);		// Pointer to end of BASIC area.
	_ram[0x009A] = 0x03;						// Current output device number. (default = screen)
	writeVec(0x00B2, 0x033C);					// Pointer to datasette buffer.
	writeVec(0x0281, 0x0800);					// Pointer to beginning of BASIC area after memory test.
	writeVec(0x0283, 0xA000);					// Pointer to end of BASIC area after memory test.
	_ram[0x0288] = 0x04;									// High byte of pointer to screen memory for screen input/output.
	writeVec(0x028F, 0xEB48);					// Execution address of routine that, based on the status of
															// shift keys, sets the pointer at memory address $00F5-$00F6
															// to the appropriate conversion table for converting keyboard matrix codes to PETSCII codes.
	writeVec(0x0300, 0xE38B);					// Execution address of warm reset, displaying optional BASIC error message and entering BASIC idle loop.

	// Hardware vectors
	writeVec(0xFFFA, 0xFE43);					// Execution address of non-maskable interrupt service routine.
	writeVec(0xFFFC, 0xFCE2);					// Execution address of cold reset.
	writeVec(0xFFFE, 0xFF48);					// Execution address of interrupt service routine.


	// create the shader
	_mainShader = std::make_unique<MainShader>(vshader, fshader, _mode);
	_mainShader->init();

	_blitShader = std::make_unique<BlitShader>(bvshader, bfshader);
	_blitShader->init();


}

std::string C64::disassemble(const OpcodeInfo& opcode, uint16_t address) {
	std::stringstream stream;
	stream << opcode.text << " ";
	switch (opcode.addressMode) {
		case AddressMode::IMPLIED:
			break;
		case AddressMode::IMMEDIATE:
			stream << "#$" << std::hex << (int) readByte(address+1);
			break;

	}
	return stream.str();
}

void C64::run() {
	bool shutdown{false};
	// main loop
	int row = 0;
	int col = 0;

	_pc = readVec(0xFFFC);

	while (!shutdown) {




		// VIC II
		for (int i = col; i < col+8; ++i) {
			_mainShader->setPixel(row, col, 0);
		}

		// 6510

		// read instruction
		auto opcode = readByte(_pc);
		const auto& op = _opcodes[opcode];
		if (op.methodPtrOne == NULL) {
			std::cout << "opcode $" << std::hex << (int)opcode << " not supported!\n";
			exit(1);
		}
		//std::cout

		// debug code
		std::cout << std::hex << (int) _pc << " ";
		for (size_t i = 0; i < 4; ++i) {
			if (i < op.bytes) {
				std::cout << std::hex << (int) readByte(_pc+i) << " ";
			} else {
				std::cout << "   ";
			}


		}
		std::cout << disassemble(op, _pc) << std::endl;
		(*this.*(op.methodPtrOne))();





		// draw
		_blitShader->start();

		_mainShader->draw();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
		glClear(GL_COLOR_BUFFER_BIT);


		glViewport(0, 0, settings::window_width, settings::window_height);
		_blitShader->draw();

		glfwSwapBuffers(window);
		glfwPollEvents();

		shutdown = (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwWindowShouldClose(window));
		_clockCycle++;
	}
}

void C64::initOpcodes() {
	_opcodes.resize(256);

	_opcodes[0xA2] = {"ldx", AddressMode::IMMEDIATE, 2, 2, &C64::ldx<2, &C64::getOperandImm>};
	_opcodes[0x78] = {"sei", AddressMode::IMPLIED, 1, 2, &C64::sei};




	//    _opcodes[0x00] = {"brk", AddressMode::IMPLIED, 1, 7, &C64::brk};
	//_opcodes[0x01] = {"ora", AddressMode::INDEXED_INDIRECT, 2, 6, &C64::ora<2, &C64::getOperandInx>};
//    _opcodes[0x05] = {"ora", AddressMode::ZEROPAGE, 2, 3, &C64::ora<2, &C64::getOperandZP>};
//    _opcodes[0x06] = {"asl", AddressMode::ZEROPAGE, 2, 5, &C64::asl<2, &C64::getRefZP>};
//    _opcodes[0x08] = {"php", AddressMode::IMPLIED, 1, 3, &C64::php};
//    _opcodes[0x09] = {"ora", AddressMode::IMMEDIATE, 2, 2, &C64::ora<2, &C64::getOperandImm>};
//    _opcodes[0x0A] = {"asl", AddressMode::ACCUMULATOR, 1, 2, &C64::asl<2, &C64::getRefAcc>};
//    _opcodes[0x0D] = {"ora", AddressMode::ABSOLUTE, 3, 4, &C64::ora<3, &C64::getOperandAbs>};
//    _opcodes[0x0E] = {"asl", AddressMode::ABSOLUTE, 3, 6, &C64::asl<3, &C64::getRefAbs>};
}


void C64::initializeGL() {
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		exit(1);
	}


	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(settings::visible_width, settings::visible_height, "EM", NULL, NULL);

	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		exit(1);
	}
	glfwMakeContextCurrent(window);
	// note: we are setting a callback for the frame buffer resize event,
	// so the dimensions we will get will be in pixels and NOT screen coordinates!
	glfwSetFramebufferSizeCallback(window, WindowResizeCallback);

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		exit(1);
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	WindowResizeCallback(window, settings::visible_width, settings::visible_height);
}

void C64::readFile(const std::string &filename, uint8_t *ptr) {
    ifstream is;
    is.open (filename.c_str(), ios::binary);
    is.seekg (0, ios::end);
    auto length = is.tellg();
    is.seekg (0, ios::beg);
    is.read ((char*)ptr, length);
    is.close();
}

C64::~C64() {
    delete[] _kernal;
    delete[] _basic;
    delete[] _charRom;
    delete[] _ram;

}


void C64::setNegFlag(const uint8_t& value) {
    if (value & 0x80) {
        _status |= 0x80;
    } else {
        _status &= 0x7F;
    }
}

void C64::setZeroFlag(const uint8_t & value) {
    if (value == 0u) {
        _status |= 0x02;
    } else {
        _status &= 0xFD;
    }
}

uint8_t C64::getBit(uint8_t value, uint8_t bit) {
    return ((value & (1 << bit)) == 0 ? 0 : 1);
}

void C64::setBit(uint8_t &ref, uint8_t value, uint8_t bit) {
    if (value == 0) {
        ref &= ~(1 << bit);
    } else {
        // set bit
        ref |= 1 << bit;
    }
}



uint8_t C64::getOperandAbx() {
    return readByte(readVec(_pc + 1) + _x);
}
uint8_t C64::getOperandAby() {
    return readByte(readVec(_pc+1) + _y);
}

uint8_t C64::getOperandZP() {
    return readByte(readByte(_pc+1));
}

uint8_t C64::getOperandZPx() {
    return readByte(readByte(_pc+1) + _x);
}

void C64::push(uint8_t byte) {
    _ram[_sp] = byte;
    _sp--;
}

void C64::pushVec(uint16_t value) {
    auto hiByte = static_cast<uint8_t>((value & 0xFF00) >> 8);
    auto loByte = static_cast<uint8_t>(value & 0x00FF);
    push(hiByte);
    push(loByte);
}


uint8_t C64::pop() {
    _sp++;
    auto byte = _ram[_sp];
    return byte;
}

uint16_t C64::popVec() {
    uint16_t loByte = pop();
    uint16_t hiByte = pop();
    return (hiByte << 8) | loByte;

}

void C64::brk() {
    // sets the break and interrupt disable flag
    _status |= 0x14;
    _pc += 2;
    pushVec(_pc);
    push(_status);
    // raise interrupt event
    _pc = readVec(0xFF48);
}

void C64::php() {
    push(_status);
    _pc += 1;
}

void C64::pha() {
    push(_a);
    _pc += 1;
}

void C64::clc() {
    _status &= 0xFE;
    _pc += 1;
}

void C64::jmp_abs() {
    _pc = readVec(_pc+1);
}

void C64::jmp_ind() {
    _pc = readVec(readVec(_pc+1));
}

void C64::sec() {
    _status |= 0x01;
    _pc += 1;
}

void C64::plp() {
    _status = pop();
    _pc += 1;
}

void C64::branch(bool value) {
    if (value) {
        uint8_t offset = readByte(_pc+1);
        int8_t signedOffset = offset;
        _pc = _pc + 2 + signedOffset;
    } else {
        _pc += 2;
    }
}

void C64::bpl() {
    branch((_status & 0x80) == 0);
}


void C64::bmi() {
    branch((_status & 0x80) != 0);
}

void C64::bvc() {
    branch((_status & 0x40) == 0);
}

void C64::bvs() {
    branch((_status & 0x40) != 0);
}

void C64::cli() {
    _status &= 0xFB;
    _pc += 1;
}
void C64::jsr() {
    uint16_t jmpAddress = readVec(_pc+1);
    _pc += 2;
    pushVec(_pc);
    _pc = jmpAddress;
}

void C64::rti() {
    _status = pop();
    _pc = popVec();
}

void C64::rts() {
    _pc = popVec();
    _pc += 1;
}

void C64::pla() {
    // pull accumulator
    _a = pop();
    _pc += 1;
}

void C64::sei() {
    _status |= 0x04;
    _pc += 1;
}


//C64::C64(Mode mode) : _mode(mode), _clockCycle(0) {
//    _kernal = new uint8_t[8192];
//    _basic = new uint8_t[8192];
//    _charRom = new uint8_t[4096];
//    _ram = new uint8_t[65536];
//
//    readFile ("/home/fabrizio/c64/rom/kernal", &_kernal[0]);
//    readFile ("/home/fabrizio/c64/rom/basic", &_basic[0]);
//    readFile ("/home/fabrizio/c64/rom/chargen", &_charRom[0]);
//    memset(_ram, 0x00, 65536);
//
//    _ram[0x0000] = 0x2F;
//    _ram[0x0001] = 0x37;
//
//    // initialize program counter
//    _pc = readVec(0xFFFC);
//
//    _opcodes[0x00] = {"brk", AddressMode::IMPLIED, 1, 7, &C64::brk};
//    _opcodes[0x01] = {"ora", AddressMode::INDEXED_INDIRECT, 2, 6, &C64::ora<2, &C64::getOperandInx>};
//    _opcodes[0x05] = {"ora", AddressMode::ZEROPAGE, 2, 3, &C64::ora<2, &C64::getOperandZP>};
//    _opcodes[0x06] = {"asl", AddressMode::ZEROPAGE, 2, 5, &C64::asl<2, &C64::getRefZP>};
//    _opcodes[0x08] = {"php", AddressMode::IMPLIED, 1, 3, &C64::php};
//    _opcodes[0x09] = {"ora", AddressMode::IMMEDIATE, 2, 2, &C64::ora<2, &C64::getOperandImm>};
//    _opcodes[0x0A] = {"asl", AddressMode::ACCUMULATOR, 1, 2, &C64::asl<2, &C64::getRefAcc>};
//    _opcodes[0x0D] = {"ora", AddressMode::ABSOLUTE, 3, 4, &C64::ora<3, &C64::getOperandAbs>};
//    _opcodes[0x0E] = {"asl", AddressMode::ABSOLUTE, 3, 6, &C64::asl<3, &C64::getRefAbs>};
//
//    _opcodes[0x10] = {"bpl", AddressMode::RELATIVE, 2, 2, &C64::bpl};
//    _opcodes[0x11] = {"ora", AddressMode::INDIRECT_INDEXED, 2, 5, &C64::ora<2, &C64::getOperandIny>};
//    _opcodes[0x15] = {"ora", AddressMode::ZEROPAGE_INDEXED, 2, 4, &C64::ora<2, &C64::getOperandZPx>};
//    _opcodes[0x16] = {"asl", AddressMode::ZEROPAGE_INDEXED, 2, 6, &C64::asl<2, &C64::getRefZPx>};
//    _opcodes[0x18] = {"clc", AddressMode::IMPLIED, 1, 2, &C64::clc};
//    _opcodes[0x19] = {"ora", AddressMode::ABSOLUTE_Y, 3, 4, &C64::ora<3, &C64::getOperandAby>};
//    _opcodes[0x1D] = {"ora", AddressMode::ABSOLUTE_X, 3, 4, &C64::ora<3, &C64::getOperandAbx>};
//    _opcodes[0x1E] = {"asl", AddressMode::ABSOLUTE_X, 3, 4, &C64::asl<3, &C64::getRefAbx>};
//
//    _opcodes[0x20] = {"jsr", AddressMode::ABSOLUTE, 3, 6, &C64::jsr};
//    _opcodes[0x21] = {"and", AddressMode::INDEXED_INDIRECT, 2, 6, &C64::_and<2, &C64::getOperandInx>};
//    _opcodes[0x24] = {"bit", AddressMode::ZEROPAGE, 2, 3, &C64::bit<2, &C64::getOperandZP>};
//    _opcodes[0x25] = {"and", AddressMode::ZEROPAGE, 2, 3, &C64::_and<2, &C64::getOperandZP>};
//    _opcodes[0x26] = {"rol", AddressMode::ZEROPAGE, 2, 5, &C64::rol<2, &C64::getRefZP>};
//    _opcodes[0x29] = {"and", AddressMode::IMMEDIATE, 2, 2, &C64::_and<2, &C64::getOperandImm>};
//    _opcodes[0x28] = {"plp", AddressMode::IMPLIED, 1, 4, &C64::plp};
//    _opcodes[0x2A] = {"rol", AddressMode::ACCUMULATOR, 1, 2, &C64::rol<1, &C64::getRefAcc>};
//    _opcodes[0x2C] = {"bit", AddressMode::ABSOLUTE, 3, 4, &C64::bit<2, &C64::getOperandAbs>};
//    _opcodes[0x2D] = {"and", AddressMode::ABSOLUTE, 3, 4, &C64::_and<2, &C64::getOperandAbs>};
//    _opcodes[0x2E] = {"rol", AddressMode::ABSOLUTE, 3, 6, &C64::rol<1, &C64::getRefAbs>};
//
//    _opcodes[0x30] = {"bmi", AddressMode::RELATIVE, 2, 2, &C64::bmi};
//    _opcodes[0x31] = {"and", AddressMode::INDIRECT_INDEXED, 2, 5, &C64::_and<2, &C64::getOperandIny>};
//    _opcodes[0x35] = {"and", AddressMode::ZEROPAGE_INDEXED, 2, 4, &C64::_and<2, &C64::getOperandZPx>};
//    _opcodes[0x36] = {"rol", AddressMode::ZEROPAGE_INDEXED, 2, 6, &C64::rol<1, &C64::getRefZPx>};
//    _opcodes[0x38] = {"sec", AddressMode::IMPLIED, 1, 2, &C64::sec};
//    _opcodes[0x39] = {"and", AddressMode::ABSOLUTE_Y, 3, 4, &C64::_and<2, &C64::getOperandAby>};
//    _opcodes[0x3D] = {"and", AddressMode::ABSOLUTE_X, 3, 4, &C64::_and<2, &C64::getOperandAbx>};
//    _opcodes[0x3E] = {"rol", AddressMode::ABSOLUTE_X, 3, 7, &C64::rol<1, &C64::getRefAbx>};
//
//    _opcodes[0x40] = {"rti", AddressMode::IMPLIED, 1, 6, &C64::rti};
//    _opcodes[0x41] = {"eor", AddressMode::INDEXED_INDIRECT, 2, 6, &C64::eor<2, &C64::getOperandInx>};
//    _opcodes[0x45] = {"eor", AddressMode::ZEROPAGE, 2, 3, &C64::eor<2, &C64::getOperandZP>};
//    _opcodes[0x46] = {"lsr", AddressMode::ZEROPAGE, 2, 5, &C64::lsr<2, &C64::getRefZP>};
//    _opcodes[0x48] = {"pha", AddressMode::IMPLIED, 1, 3, &C64::pha};
//    _opcodes[0x49] = {"eor", AddressMode::IMMEDIATE, 2, 2, &C64::eor<2, &C64::getOperandImm>};
//    _opcodes[0x4A] = {"lsr", AddressMode::ACCUMULATOR, 1, 2, &C64::lsr<2, &C64::getRefAcc>};
//    _opcodes[0x4C] = {"jmp", AddressMode::ABSOLUTE, 3, 3, &C64::jmp_abs};
//    _opcodes[0x4D] = {"eor", AddressMode::ABSOLUTE, 3, 4, &C64::eor<3, &C64::getOperandAbs>};
//    _opcodes[0x4E] = {"lsr", AddressMode::ABSOLUTE, 3, 6, &C64::lsr<3, &C64::getRefAbs>};
//
//    _opcodes[0x50] = {"bvc", AddressMode::RELATIVE, 2, 2, &C64::bvc};
//    _opcodes[0x51] = {"eor", AddressMode::INDIRECT_INDEXED, 2, 5, &C64::eor<2, &C64::getOperandIny>};
//    _opcodes[0x55] = {"eor", AddressMode::ZEROPAGE_INDEXED, 2, 4, &C64::eor<2, &C64::getOperandZPx>};
//    _opcodes[0x56] = {"lsr", AddressMode::ZEROPAGE_INDEXED, 2, 6, &C64::lsr<2, &C64::getRefZPx>};
//    _opcodes[0x58] = {"cli", AddressMode::IMPLIED, 1, 1, &C64::cli};
//    _opcodes[0x59] = {"eor", AddressMode::ABSOLUTE_Y, 3, 4, &C64::eor<3, &C64::getOperandAby>};
//    _opcodes[0x5D] = {"eor", AddressMode::ABSOLUTE_X, 3, 4, &C64::eor<3, &C64::getOperandAbx>};
//    _opcodes[0x5E] = {"lsr", AddressMode::ABSOLUTE_X, 3, 7, &C64::lsr<3, &C64::getRefAbx>};
//
//    _opcodes[0x60] = {"rts", AddressMode::IMPLIED, 1, 6, &C64::rts};
//    _opcodes[0x61] = {"adc", AddressMode::INDEXED_INDIRECT, 2, 6, &C64::adc<2, &C64::getOperandInx>};
//    _opcodes[0x65] = {"adc", AddressMode::ZEROPAGE, 2, 3, &C64::adc<2, &C64::getOperandZP>};
//    _opcodes[0x66] = {"rol", AddressMode::ZEROPAGE, 2, 5, &C64::ror<2, &C64::getRefZP>};
//    _opcodes[0x68] = {"pla", AddressMode::IMPLIED, 1, 4, &C64::pla};
//    _opcodes[0x69] = {"adc", AddressMode::IMMEDIATE, 2, 2, &C64::adc<2, &C64::getOperandImm>};
//    _opcodes[0x6A] = {"ror", AddressMode::ACCUMULATOR, 1, 2, &C64::ror<1, &C64::getRefAcc>};
//    _opcodes[0x6C] = {"jmp", AddressMode::ABSOLUTE, 3, 5, &C64::jmp_ind};
//    _opcodes[0x6D] = {"adc", AddressMode::ABSOLUTE, 3, 4, &C64::adc<3, &C64::getOperandAbs>};
//    _opcodes[0x6E] = {"ror", AddressMode::ABSOLUTE, 3, 6, &C64::ror<1, &C64::getRefAbs>};
//
//    _opcodes[0x70] = {"bvs", AddressMode::RELATIVE, 2, 2, &C64::bvs};
//    _opcodes[0x71] = {"adc", AddressMode::INDIRECT_INDEXED, 2, 5, &C64::adc<2, &C64::getOperandIny>};
//    _opcodes[0x75] = {"adc", AddressMode::ZEROPAGE_INDEXED, 2, 4, &C64::adc<2, &C64::getOperandZPx>};
//    _opcodes[0x76] = {"ror", AddressMode::ZEROPAGE_INDEXED, 2, 6, &C64::ror<1, &C64::getRefZPx>};
//    _opcodes[0x78] = {"sei", AddressMode::IMPLIED, 1, 2, &C64::sei};
//    _opcodes[0x79] = {"adc", AddressMode::ABSOLUTE_Y, 3, 4, &C64::adc<3, &C64::getOperandAby>};
//    _opcodes[0x7D] = {"adc", AddressMode::ABSOLUTE_X, 3, 4, &C64::adc<3, &C64::getOperandAbx>};
//    _opcodes[0x7E] = {"ror", AddressMode::ABSOLUTE_X, 3, 7, &C64::ror<1, &C64::getRefAbx>};
//
//    _opcodes[0x81] = {"sta", AddressMode::INDEXED_INDIRECT, 2, 6, &C64::sta<2, &C64::getRefInx>};
//    _opcodes[0x84] = {"sty", AddressMode::ZEROPAGE, 2, 3, &C64::sty<2, &C64::getRefZP>};
//    _opcodes[0x85] = {"sta", AddressMode::ZEROPAGE, 2, 3, &C64::sta<2, &C64::getRefZP>};
//    _opcodes[0x86] = {"stx", AddressMode::ZEROPAGE, 2, 3, &C64::stx<2, &C64::getRefZP>};
//
//    //std::vector<uint8_t> data((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
//    //FILE *pFile;
//    //pFile = fopen ("C:\\myfile.gif", "w");
//    //fwrite (buffer , 1 , sizeof(buffer) , pFile );
//
//}

uint8_t & C64::getRefAbs() {
    return _ram[readVec(_pc+1)];
}
uint8_t & C64::getRefAbx() {
    return _ram[readVec(_pc+1) + _x];
}

uint8_t & C64::getRefZP() {
    return _ram[readByte(_pc+1)];
}
uint8_t & C64::getRefZPx() {
    return _ram[readByte(_pc+1)+_x];
}
uint8_t C64::getOperandImm() {
    return readByte(_pc+1);
}

uint8_t C64::getOperandAbs() {
    return readByte(readVec(_pc+1));
}

uint8_t C64::getOperandInx() {
    return readByte(readByte(_pc+1) + _x);
}

uint8_t & C64::getRefInx() {
    return _ram[readVec(readByte(_pc+1) + _x)];
}

uint8_t & C64::getRefIny() {

}
uint8_t C64::getOperandIny() {
    return readByte(readVec(readByte(_pc+1)) + _y);
}



uint16_t C64::strToVec(const std::string & s) {
    if (s[0] == '$') {
        return std::stoul(s.substr(1).c_str(), nullptr, 16);
    }
    return std::stoul(s.c_str(), nullptr, 10);
}


uint8_t * C64::getPtr(uint16_t address) const {
	// Configuration for memory areas $A000-$BFFF, $D000-$DFFF and $E000-$FFFF depends
	// on the 3 LSB of processor port (0x0001)
	// 0xA0000-0xBFFFF
    if (address >= 0xA000 && address <= 0xBFFF) {
        if (_ram[0x1] & 0x03) {
            // basic
            return &(_basic[address - 0xA0000]);
        }
        return &(_ram[address]);
    }

    // 0xD000-0xDFFF
    if (address >= 0xD000 && address <= 0xDFFF) {
    	if ((_ram[0x0001] & 0x03u) == 0u) {
    		return &_ram[address];
    	} else {
    		if ((_ram[0x0001] & 0x04) == 0) {
    			// character ROM
    			return &(_charRom[address - 0xD000]);
    		} else {
    			// I/O registers
    			if (address < 0xD400) {
    				// VIC-II video display
    				uint16_t a = (address - 0xD000) / 64;
    				return _vic->getPtr(a);
    			} else {
    				// SID
    			}
    		}
    	}
    }

    if (address >= 0xE000 && address <= 0xFFFF) {
        if (_ram[0x1] & (1<<1)) {
            return &_kernal[address - 0xE000];
        }
        return &_ram[address];
    }

    return &_ram[address];

}

uint8_t C64::readByte(uint16_t address) const {
    return *(getPtr(address));
}

void C64::writeVec(uint16_t address, uint16_t value) {
	_ram[address] = (value & 0x00FF);
	_ram[address+1] = (value >> 8);
}

uint16_t C64::readVec(uint16_t address) const {
    auto* ptr = getPtr(address);
    return 256 * *(ptr+1) + *ptr;
}

//void C64::run() {
//    high_resolution_clock::time_point t1 = high_resolution_clock::now();
//
//    duration<double> period(1.0 / CLOCK_FREQUENCY[_mode]);
//    high_resolution_clock::time_point t0 = high_resolution_clock::now();
//    int i = 0;
//
//    // number of cycles for current instruction
//    int numberOfCycles{0};
//    int currentCycle{0};
//
//    while (i < 2) {
//
//        // CYCLE
//        if (currentCycle >= numberOfCycles) {
//            // fetch new instruction
//            std::cout << " current PC: " << std::hex << _pc << std::endl;
//            std::cout << " instruction: " << (int) this->readByte(_pc) << std::endl;
//
//        }
//
//        // vic ii phase ... put 8 pixels
//
//
//
//        exit(1);
//
//    }
//
//    high_resolution_clock::time_point t0e = high_resolution_clock::now();
//    duration<double> time_span = duration_cast<duration<double>>(t0e - t0);
//    std::cout << "run for " << time_span.count() << "\n";
//}


void C64::test() {
    std::string cmd;
    while (true) {
        std::string line;
        getline(cin, line, '\n');
        std::stringstream stream(line);
        std::string c, arg1;
        stream >> c;
        if (c=="x") break;
        if (c=="m") {
            stream >> arg1;
            auto address =strToVec(arg1);
            auto byte = readByte(address);
            std::cout << "m(" << address << ") = " << (int)byte << " $" << std::hex << (int)byte << std::dec << "\n";
        } else if (c=="M") {
            stream >> arg1;
            auto address =strToVec(arg1);
            auto byte = readVec(address);
            std::cout << "m(" << address << ") = " << (int)byte << " $" << std::hex << (int)byte << std::dec << "\n";
        }

    }
}
//
//uint16_t C64::sign_extend(uint8_t u) {
//    if (u & 0x80) {
//        return 0xFF00 + u;
//    } else {
//        return u;
//    }
//}
//
//bool C64::isDecimalMode() const {
//    return m_sr & 0x08;
//}
//
//
//void C64::setNegativeFlag(uint8_t b) {
//    if (b & 0x80) {       // negative
//        m_sr |= 0x80;
//    } else {
//        m_sr &= 0x7F;
//    }
//}
//
//void C64::setCarryFlag(bool value) {
//    if (value) {
//        m_sr |= 0x01u;
//    } else {
//        m_sr &= 0xFEu;
//    }
//}
//
//void C64::setOverflowFlag(bool value) {
//    if (value) {
//        m_sr |= 0x40u;
//    } else {
//        m_sr &= 0xDFu;
//    }
//}
//
//void C64::setZeroFlag(uint8_t b) {
//    if (b == 0) {
//        m_sr |= 0x02;
//    } else {
//        m_sr &= 0xFD;
//    }
//}
//
//// indexed-indirect addressing X
//uint8_t C64::addr_inx(uint8_t offset) {
//    uint8_t zpa = m_x + offset;
//    return m_memory[vecAsInt(zpa)];
//}
//
//// indexed-indirect addressing Y
//uint8_t C64::addr_iny(uint8_t offset) {
//    uint8_t zpa = m_y + offset;
//    return m_memory[vecAsInt(zpa)];
//}
//
//uint16_t C64::vecAsInt(uint16_t address) {
//    return m_memory[address] + m_memory[address+1] * 256;
//}
//
//Vector C64::getVec(uint16_t i) {
//    return Vector{m_memory[i], m_memory[i+1]};
//}
//
//uint16_t Vector::toInt() const {
//    return lo + hi * 256;
//}
//
///* CLC (short for "CLear Carry") is the mnemonic for a machine language instruction which unconditionally clears the
// * carry flag.
//*/
//void C64::clc(uint16_t) {
//    m_sr &= 0xFE;
//}
//
///* SEC (short for "SEt Carry") is the mnemonic for a machine language instruction which unconditionally sets the carry
// * flag.
// */
//void C64::sec(uint16_t) {
//    m_sr |= 0x01;
//}
//
///* PHP (short for "PusH Processor flags") is the mnemonic for a machine language instruction which stores the current
// * state of the processor status flags onto the stack, and adjusting the stack pointer to reflect the addition.
// */
//void C64::php(uint16_t) {
//    m_memory[m_sp] = m_sr;
//    m_sp--;
//    // TODO check stack overflow
//
//}
//
///* PLP (short for "PulL Processor flags") is the mnemonic for a machine language instruction which retrieves a set
// * of status flags previously "pushed" onto the stack (usually by a PHP instruction) from the stack, and adjusting
// * the stack pointer to reflect the removal of a byte.
// */
//void C64::plp(uint16_t) {
//    m_sp++;
//    m_sr = m_memory[m_sp];
//    // TODO check stack underflow
//}
//
///* PHA (short for "PusH Accumulator") is the mnemonic for a machine language instruction which stores a copy of the
// * current content of the accumulator onto the stack, and adjusting the stack pointer to reflect the addition.
// */
//void C64::pha(uint16_t) {
//    m_memory[m_sp] = m_a;
//    m_sp--;
//    // TODO check stack overflow
//}
//
///* PLA (short for "PulL Accumulator") is the mnemonic for a machine language instruction which retrieves a byte from
// * the stack and stores it in the accumulator, and adjusts the stack pointer to reflect the removal of that byte.
// */
//void C64::pla(uint16_t) {
//    m_sp++;
//    m_a = m_memory[m_sp];
//
//    // The negative status flag is set if the retrieved byte is negative, i.e. has its most significant bit set.
//    setNegativeFlag(m_a);
//
//    // The zero flag is set if the retrieved byte is zero, or cleared if it is non-zero.
//    setZeroFlag(m_a);
//}
//
///* CLI (short for "CLear Interrupt flag") is the mnemonic for a machine language instruction which clears the interrupt
// * flag, so that the CPU will respond to IRQ interrupt events. To disable the response to IRQs, use the complementary
// * instruction SEI.
// */
//void C64::cli(uint16_t) {
//    m_sr &= 0xFB;
//}
//
///* SEI (short for "SEt Interrupt flag") is the mnemonic for a machine language instruction which sets the interrupt
// * flag, thereby preventing the CPU from responding to IRQ interrupt events. To re-enable the response to IRQs, use the
// * complementary instruction CLI.
// */
//void C64::sei(uint16_t) {
//    m_sr |= 0x04;
//    m_pc++;
//}
//
///* DEY (short for "DEcrease Y") is the mnemonic for a machine language instruction which decrements the numerical value
// * of Y index register, by one, and "wraps over" if the value goes below the numerical limits of a byte.
// */
//void C64::dey(uint16_t) {
//    m_y--;
//    setNegativeFlag(m_y);
//    setZeroFlag(m_y);
//}
//
///* TYA (short for "Transfer Y to Accumulator") is the mnemonic for a machine language instruction which transfers
// * ("copies") the contents of the Y index register into the accumulator.
// */
//void C64::tya(uint16_t) {
//    m_a = m_y;
//
//    // The negative status flag is set if the byte transferred is negative, i.e. has it's most significant bit set.
//    setNegativeFlag(m_a);
//
//    // The zero flag is set if the result is zero, or cleared if it is non-zero.
//    setZeroFlag(m_a);
//}
//
///* TAY (short for "Transfer Accumulator to Y") is the mnemonic for a machine language instruction which transfers
// * ("copies") the contents of the accumulator into the Y index register.
// */
//void C64::tay(uint16_t) {
//   m_y = m_a;
//
//   // The negative status flag is set if the byte transferred is negative, i.e. has it's most significant bit set.
//   setNegativeFlag(m_y);
//
//   // The zero flag is set if the byte transferred is zero, or cleared if it is non-zero.
//   setZeroFlag(m_y);
//
//}
//
///* CLV (short for "CLear oVerflow") is the mnemonic for a machine language instruction which clears the Overflow flag.
// */
//void C64::clv(uint16_t) {
//    m_sr &= 0xBF;
//}
//
///* INY (short for "INcrease Y") is the mnemonic for a machine language instruction which increases the numerical value
// * held in the Y index register by one, and "wraps over" when the numerical limits of a byte are exceeded.
// */
//void C64::iny(uint16_t) {
//    m_y++;
//
//    // The negative status flag is set if the result is negative, i.e. has its most significant bit set.
//    setNegativeFlag(m_y);
//
//    // The zero flag is set if the result is zero, or cleared if it is non-zero.
//    setZeroFlag(m_y);
//
//    m_pc++;
//}
//
///* CLD (short for "CLear Decimal flag") is the mnemonic for a machine language instruction which clears the decimal
// * flag. It complements the SED (set decimal flag) instruction.
// */
//void C64::cld(uint16_t) {
//   m_sr &= 0xF7;
//}
//
///* INX (short for "INcrease X") is the mnemonic for a machine language instruction which increases the numerical
// * value held in the X index register by one, and "wraps over" when the numerical limits of a byte are exceeded.
// */
//void C64::inx(uint16_t) {
//    m_x++;
//
//    // The negative status flag is set if the result is negative, i.e. has its most significant bit set.
//    setNegativeFlag(m_x);
//
//    // The zero flag is set if the result is zero, or cleared if it is non-zero.
//    setZeroFlag(m_x);
//}
//
///* SED (short for "SEt Decimal flag") is the mnemonic for a machine language instruction which sets the decimal flag.
// * It complements the CLD (clear decimal flag) instruction.
// */
//void C64::sed(uint16_t) {
//    m_sr |= 0x08;
//}
//
//void C64::ora_imm(uint16_t i) {
//    m_a |= m_memory[i+1];
//    setNegativeFlag(m_a);
//    setZeroFlag(m_a);
//}
//
//void C64::ora_abs(uint16_t i) {
//    auto addr = getVec(i+1).toInt();
//    m_a |= m_memory[addr];
//    setNegativeFlag(m_a);
//    setZeroFlag(m_a);
//}
//
//void C64::ora_abx(uint16_t i) {
//    auto addr = getVec(i+1).toInt() + m_x;
//    m_a |= m_memory[addr];
//    setNegativeFlag(m_a);
//    setZeroFlag(m_a);
//}
//
//void C64::ora_aby(uint16_t i) {
//    auto addr = getVec(i+1).toInt() + m_y;
//    m_a |= m_memory[addr];
//    setNegativeFlag(m_a);
//    setZeroFlag(m_a);
//}
//
//void C64::ora_zp(uint16_t i) {
//    m_a |= m_memory[m_memory[i+1]];
//    setNegativeFlag(m_a);
//    setZeroFlag(m_a);
//}
//
//void C64::ora_zpx(uint16_t i) {
//    auto addr = m_memory[i+1] + m_x;
//    m_a |= m_memory[addr];
//    setNegativeFlag(m_a);
//    setZeroFlag(m_a);
//}
//
//void C64::ora_inx(uint16_t i) {
//    m_a |= addr_inx(m_memory[i+1]);
//    setNegativeFlag(m_a);
//    setZeroFlag(m_a);
//}
//
//void C64::ora_iny(uint16_t i) {
//    m_a |= addr_iny(m_memory[i+1]);
//    setNegativeFlag(m_a);
//    setZeroFlag(m_a);
//}
//
//void C64::and_imm(uint16_t i) {
//    m_a &= m_memory[i+1];
//    setNegativeFlag(m_a);
//    setZeroFlag(m_a);
//}
//
//void C64::and_abs(uint16_t i) {
//    auto addr = getVec(i+1).toInt();
//    m_a &= m_memory[addr];
//    setNegativeFlag(m_a);
//    setZeroFlag(m_a);
//}
//
//void C64::and_abx(uint16_t i) {
//    auto addr = getVec(i+1).toInt() + m_x;
//    m_a &= m_memory[addr];
//    setNegativeFlag(m_a);
//    setZeroFlag(m_a);
//}
//
//void C64::and_aby(uint16_t i) {
//    auto addr = getVec(i+1).toInt() + m_y;
//    m_a &= m_memory[addr];
//    setNegativeFlag(m_a);
//    setZeroFlag(m_a);
//}
//
//void C64::and_zp(uint16_t i) {
//    m_a &= m_memory[m_memory[i+1]];
//    setNegativeFlag(m_a);
//    setZeroFlag(m_a);
//}
//
//void C64::and_zpx(uint16_t i) {
//    m_a &= m_memory[m_memory[i+1] + m_x];
//    setNegativeFlag(m_a);
//    setZeroFlag(m_a);
//}
//
//void C64::and_inx(uint16_t i) {
//    m_a &= addr_inx(m_memory[i+1]);
//    setNegativeFlag(m_a);
//    setZeroFlag(m_a);
//}
//
//void C64::and_iny(uint16_t i) {
//    m_a &= addr_iny(m_memory[i+1]);
//    setNegativeFlag(m_a);
//    setZeroFlag(m_a);
//}
//
//void C64::eor_imm(uint16_t i) {
//    m_a ^= m_memory[i+1];
//    setNegativeFlag(m_a);
//    setZeroFlag(m_a);
//}
//
//void C64::eor_abs(uint16_t i) {
//    auto addr = getVec(i+1).toInt();
//    m_a ^= m_memory[addr];
//    setNegativeFlag(m_a);
//    setZeroFlag(m_a);
//}
//
//void C64::eor_abx(uint16_t i) {
//    auto addr = getVec(i+1).toInt() + m_x;
//    m_a ^= m_memory[addr];
//    setNegativeFlag(m_a);
//    setZeroFlag(m_a);
//}
//
//void C64::eor_aby(uint16_t i) {
//    auto addr = getVec(i+1).toInt() + m_y;
//    m_a ^= m_memory[addr];
//    setNegativeFlag(m_a);
//    setZeroFlag(m_a);
//}
//
//void C64::eor_zp(uint16_t i) {
//    m_a ^= m_memory[m_memory[i+1]];
//    setNegativeFlag(m_a);
//    setZeroFlag(m_a);
//}
//
//void C64::eor_zpx(uint16_t i) {
//    m_a ^= m_memory[m_memory[i+1] + m_x];
//    setNegativeFlag(m_a);
//    setZeroFlag(m_a);
//}
//
//void C64::eor_inx(uint16_t i) {
//    m_a ^= addr_inx(m_memory[i+1]);
//    setNegativeFlag(m_a);
//    setZeroFlag(m_a);
//}
//
//void C64::eor_iny(uint16_t i) {
//    m_a ^= addr_iny(m_memory[i+1]);
//    setNegativeFlag(m_a);
//    setZeroFlag(m_a);
//}
//
//void C64::adc_imm(uint16_t i) {
//    if (isDecimalMode()) {
//        // TODO
//    } else {
//        // get current carry
//        uint8_t carry = m_sr & 0x01;
//        uint16_t tmp = m_a + carry + m_memory[i + 1];
//        // the lsb goes to a, the msb holds info about carry
//        uint8_t tmp2 = tmp & 0x00FFu;
//        bool overflow = (((m_a ^ m_memory[i + 1]) & 0x80u) == 0) && (((m_a ^ tmp2) & 0x80u) == 1);
//        m_a = tmp2;
//        uint8_t nc = tmp & 0x0100u;
//        setCarryFlag(nc != 0);
//        setOverflowFlag(overflow);
//        setZeroFlag(m_a);
//        setNegativeFlag(m_a);
//
//    }
//
//}
//
//
//void C64::setProgramCounter(uint16_t i) {
//    m_pc = i;
//}
//void C64::step() {
//    uint8_t opcode = m_memory[m_pc];
//    std::cout << m_pc << " opcode : " << int(opcode) << " ";
//    const auto& info = m_opcodeInfos[opcode];
//    for (auto i = 1; i < info.bytes; ++i) {
//        std::cout << int(m_memory[m_pc+i]) << " ";
//    }
//    std::cout << "\n";
//    auto f = m_opcodes[opcode];
//    if (f) {
//        f(m_pc);
//    } else {
//        std::cout << " !! unknown opcode !!\n";
//        exit(1);
//    }
//
//}
//
//void C64::exec(uint16_t i) {
//    // set the program counter
//    m_pc = i;
//    bool continueLoop = true;
//    int co = 0;
//    while (co < 20000) {
//        // fetch the opcode
//        uint8_t opcode = m_memory[m_pc];
//        std::cout << m_pc << " opcode : " << int(opcode) << " ";
//        const auto& info = m_opcodeInfos[opcode];
//        for (auto i = 1; i < info.bytes; ++i) {
//            std::cout << int(m_memory[m_pc+i]) << " ";
//        }
//        std::cout << "\n";
//        auto f = m_opcodes[opcode];
//        if (f) {
//            f(m_pc);
//        } else {
//            std::cout << " !! unknown opcode !!\n";
//            exit(1);
//        }
//        co++;
//    };
//}
//
//void C64::load_prg(const std::vector<uint8_t> & data) {
//    // address is contained in the first 2 bytes
//    uint16_t addr = data[0] + data[1] * 256;
//    std::cout << "storing program @ " << addr << std::endl;
//    for (size_t i = 2; i < data.size(); ++i) {
//        m_memory[addr++] = data[i];
//    }
//}
//
//void C64::lda_imm(uint16_t i) {
//    m_a = m_memory[i+1];
//    setNegativeFlag(m_a);
//    setZeroFlag(m_a);
//    m_pc += 2;
//}
//
//void C64::lda_aby(uint16_t i) {
//    auto addr = getVec(i+1).toInt() + m_y;
//    m_a = m_memory[addr];
//    setNegativeFlag(m_a);
//    setZeroFlag(m_a);
//    m_pc += 3;
//
//
//}
//
//void C64::ldy_imm(uint16_t i) {
//    m_y = m_memory[i+1];
//    setNegativeFlag(m_y);
//    setZeroFlag(m_y);
//    m_pc += 2;
//}
//
//void C64::sta_aby(uint16_t i) {
//    auto addr = getVec(i+1).toInt() + m_y;
//    m_memory[addr] = m_a;
//    m_pc += 3;
//}
//
//void C64::sta_zp(uint16_t i) {
//    m_memory[m_memory[i+1]] = m_a;
//    m_pc += 2;
//}
//
//void C64::sty_zp(uint16_t i) {
//    m_memory[m_memory[i+1]] = m_y;
//    m_pc += 2;
//}
//
//void C64::bne(uint16_t i) {
//    if ((m_sr & 0x02) == 0) {
//        // zero flag is clear --> jump
//        uint8_t offset = m_memory[i+1];
//
//        m_pc += 2 + sign_extend(m_memory[i+1]);
//    } else {
//        m_pc += 2;
//    }
//}
//
//void C64::jmp(uint16_t i) {
//    auto addr = getVec(i+1).toInt();
//    m_pc = addr;
//
//}
//
//void C64::inc_abs(uint16_t i) {
//    auto addr = getVec(i+1).toInt();
//    m_memory[addr]++;
//    m_pc += 3;
//}
//
//uint8_t C64::peek(uint16_t i) {
//    return m_memory[i];
//}
//
//void C64::poke(uint16_t address, uint8_t value) {
//    m_memory[address] = value;
//}
//
//void C64::poke(uint16_t address, const std::vector<uint8_t>& data) {
//    uint16_t a = address;
//    for (const auto& b : data) {
//        m_memory[a++] = b;
//    }
//}