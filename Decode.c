#include "Emulator.h"

unsigned short opcode_3h, opcode_3l, opcode_4h, opcode_4l;

void branch_link_decode(unsigned short opcode, unsigned short* offset) {
	//LR = PC; //link register gets program counter value

	*offset = offsetB0_12(IR); //intruction masked to get only the offset
	if ((*offset & 0x1000) == 0x1000) // sign extension only if offset is negative
		*offset = *offset | 0xE000;
	*offset = *offset << 1; //shift offset to left to make it even

	table_index = 0;
}

void branch_with_conditions_decode(unsigned short opcode, unsigned short* offset) {
	enum bool match = FALSE;
	int i = 0;

	opcode = B10_15(IR);

	*offset = offsetB0_9(IR); //intruction masked to get only the offset
	if ((*offset & 0x0200) == 0x0200) // sign extension only if offset is negative
		*offset = *offset | 0xFC00;
	*offset = *offset << 1; //shift offset to left to make it even

	for (i; i < branch_sz; i++) {
		if ((opcode == branch_table[i].opcode) && (branch_table[i].name != "BL")) {
			match = TRUE; //instruction found
		}
		if (match == TRUE) {
			table_index = i; //store instruction index of table for execute to use
			break;
		}
	}

	if (!match) { //match/instruction not found
		if (word_psw.psw.faulting == set) {
			//double fault
			double_fault = set;
			printf("\n[Double fault ($11)!!]");
		}
		else {
			word_psw.psw.faulting = set; //illegal instruction fault
		}
		unsigned short pc = PC;
		printf("\n[Illegal instruction ($8)!! Instruction register is empty: 0x%04x at address: 0x%04x]\n", IR, pc - 2);
		word_psw.psw.faulting = unset;
	}
}

void arithmetic_decode(unsigned short opcode, unsigned short* rc, unsigned short* wb, unsigned short* sc, unsigned short* d) {
	enum bool match = FALSE;
	int i = 0;

	opcode = (IR >> 8) & 0x00FF;

	*rc = B(IR, 7);  // The rc bit is at position 7.
	*wb = B(IR, 6);  // The wb bit is at position 6.
	*sc = B3_5(IR); // The soucre bits
	*d = B0_2(IR);  // The d bits are in the last 3 bits.

	for (i; i < arith_sz; i++) {
		if (opcode == arithmetic_table[i].opcode) {
			match = TRUE; //found instruction
		}
		if (match == TRUE) {
			table_index = i; //store instruction index of table for execute to use
			break;
		}
	}

	if (!match) { //match/instruction not found
		if (word_psw.psw.faulting == set) {
			//double fault
			double_fault = set;
			printf("\n[Double fault ($11)!!]");
		}
		else {
			word_psw.psw.faulting = set; //illegal instruction fault
		}
		unsigned short pc = PC;
		printf("\n[Illegal instruction ($8)!! Instruction register is empty: 0x%04x at address: 0x%04x]\n", IR, pc - 2);
		word_psw.psw.faulting = unset;
	}
}

void init_register_decode(unsigned short opcode, unsigned short* prpo, unsigned short* dec, unsigned short* inc, unsigned short* wb, unsigned short* sc, unsigned short* d) {
	enum bool match = FALSE;
	int i = 0;

	*prpo = B(IR, 9); //prepost bit
	*dec = B(IR, 8); //decrement bit
	*inc = B(IR, 7); //increment bit
	*wb = B(IR, 6); //wordbyte bit

	*d = B0_2(IR); //destination bits
	*sc = B3_5(IR); //source bits

	if (B(IR, 15)) { //check if MSBit is set
		opcode = B14_15(IR); //get 2 MSBits for LDR and STR opcode
	}
	else {
		opcode = B11_15(IR); //get first 5 MSBits for opcode for MOVL to MOVH
	}

	for (i; i < init_reg_sz; i++) {
		if (opcode == init_register_table[i].opcode) {
			match = TRUE; //found instruction
		}
		if (match == TRUE) {
			table_index = i; //store instruction index of table for execute to use
			break;
		}
	}

	if (!match) { //match/instruction not found
		if (word_psw.psw.faulting == set) {
			//double fault
			double_fault = set;
			printf("\n[Double fault ($11)!!]");
		}
		else {
			word_psw.psw.faulting = set; //illegal instruction fault
		}
		unsigned short pc = PC;
		printf("\n[Illegal instruction ($8)!! Instruction register is empty: 0x%04x at address: 0x%04x]\n", IR, pc - 2);
		word_psw.psw.faulting = unset;
	}
}

void reg_operation_decode(unsigned short opcode, unsigned short* prpo, unsigned short* dec, unsigned short* inc, unsigned short* wb, unsigned short* sc, unsigned short* d) {
	enum bool match = FALSE;
	int i = 0;

	*wb = B(IR, 6); //wordbyte bit
	*prpo = B(IR, 9);
	*inc = B(IR, 7);
	*dec = B(IR, 8);
	*d = B0_2(IR); //destination bits
	*sc = B3_5(IR); //source bits

	opcode = B11_15(IR);
	switch (opcode) {
	case 0x000B: //instruction LD and ST
		opcode = B10_15(IR);
		break;
	case 0x000A: //instruction CEX
		opcode = B10_15(IR);
		break;
	default: //instruction from MOV to SXT
		opcode = B8_15(IR); //get 8 MSBits of opcode
		switch (opcode) {
		case 0x004C: //instruction either MOV or SWAP
			opcode = B7_15(IR);
			break;
		default: //instruction from SRA to CLRCC
			opcode = B7_15(IR);
			switch (opcode) {
			case 0x009A: //SRA to SXT
				opcode = B3_15_mod(IR); //get 13 MSBits. does not include the word_byte bit. not taken into consideration in opcode. w_b = 0
				break;
			default: //SETPRI to CLRCC
				opcode = B6_15(IR);
				switch (opcode) {
				case 0x0136:
					opcode = B5_15(IR);
					if(opcode == 0x26C)
						opcode = B4_15(IR); //SETPRI and SVC. get 13 MSBits and set the SA bit in SVC to zero for simplicity
					else
						opcode = B3_15_mod2(IR); //SETCC and CLRCC. get 13 MSBits and set overflow(V) and sleep(SLP) bits to zero for simplicity
					break;
				case 0x137:
					opcode = B3_15_mod2(IR); //SETCC and CLRCC. get 13 MSBits and set overflow(V) and sleep(SLP) bits to zero for simplicity
					break;
				default:
					opcode = B4_15(IR); //SETPRI and SVC. get 13 MSBits and set the SA bit in SVC to zero for simplicity
					break;
				}
				break;
			}
			break;
		}
		break;
	}

	for (i; i < reg_op_sz; i++) {
		if (opcode == reg_operation_table[i].opcode) {
			match = TRUE; //found instruction
		}
		if (match == TRUE) {
			table_index = i; //store instruction index of table for execute to use
			break;
		}
	}

	if (!match) { //match/instruction not found
		if (word_psw.psw.faulting == set) {
			//double fault
			double_fault = set;
			printf("\n[Double fault ($11)!!]");
		}
		else {
			word_psw.psw.faulting = set; //illegal instruction fault
		}
		unsigned short pc = PC;
		printf("\n[Illegal instruction ($8)!! Instruction register is empty: 0x%04x at address: 0x%04x]\n", IR, pc - 2);
		word_psw.psw.faulting = unset;
	}
}