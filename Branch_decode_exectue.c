#include "Emulator.h"

void BL(unsigned short opcode, signed short offset) {
	LR = PC; // Store the return address
	PC += offset; // Branch to the target address
}

void BEQ(unsigned short opcode, signed short offset) {
	if (word_psw.psw.z == TRUE) {  // assuming you have a Zero flag in your PSW
		PC += offset;
	}
}

void BRA(unsigned short opcode, signed short offset) {
	PC += offset;
}

void BLT(unsigned short opcode, signed short offset) {
	if (word_psw.psw.n == TRUE) {
		PC += offset;
	}
}

void BNE(unsigned short opcode, signed short offset) {
	if (word_psw.psw.z == FALSE) { // if Zero flag is not set
		PC += offset;
	}
}

void BC(unsigned short opcode, signed short offset) {
	if (word_psw.psw.c == TRUE) { // if Carry flag is set
		PC += offset;
	}
}

void BNC(unsigned short opcode, signed short offset) {
	if (word_psw.psw.c == FALSE) { // if Carry flag is not set
		PC += offset;
	}
}

void BN(unsigned short opcode, signed short offset) {
	if (word_psw.psw.n == TRUE) { // if Negative flag is set
		PC += offset;
	}
}

void BGE(unsigned short opcode, signed short offset) {
	if (word_psw.psw.n == FALSE) { // if Negative flag is not set
		PC += offset;
	}
}

I_set branch_table[branch_sz] = {
	{"BL", 0x0000, BL}, {"BEQ/BZ", 0x0008, BEQ}, {"BNE/BNZ", 0x0009, BNE},
	{"BC/BHS", 0x000A, BC}, {"BNC/BLO", 0x000B, BNC}, {"BN", 0x000C, BN},
	{"BGE", 0x000D, BGE}, {"BLT", 0x000E, BLT}, {"BRA", 0x000F, BRA}
};