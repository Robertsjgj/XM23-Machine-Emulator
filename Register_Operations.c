#include "Emulator.h"

union MEMORY memory;
enum bool cond_state = FALSE; //cond_state starting at false
unsigned short true_count;
unsigned short false_count;
int double_fault;
union psw_union ISR_state;
union psw_union ISR_address;
unsigned short Vector_offset[2][2][2][2] = {0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60 };

//unsigned change_word_psw.psw[2][2];

void MOV(unsigned short source, unsigned short destination) {
	regfile[0][destination] = regfile[0][source];
}

void SWAP(unsigned short source, unsigned short destination) {
	unsigned short temp = regfile[0][source];
	regfile[0][source] = regfile[0][destination];
	regfile[0][destination] = temp;
}

void SRA(unsigned short source, unsigned short destination, unsigned short prpo, unsigned short w_b) {
	union shortchar result;
	result.word = regfile[0][0];
	if (w_b)
		regfile[0][destination] = regfile[0][destination] >> 1;
	else
		regfile[0][destination] = result.byte[0];
}

void RRC(unsigned short source, unsigned short destination, unsigned short prpo, unsigned short w_b) {
	union shortchar result;
	result.word = regfile[0][1];
	if (w_b) {
		unsigned short temp = (word_psw.psw.c << 15) & 0x8000;
		word_psw.psw.c = regfile[0][destination] & 0x0001;
		regfile[0][destination] = ((regfile[0][destination] >> 1) & 0x7FFF) + temp;
	}
	else {
		regfile[0][destination] = result.byte[0];
	}
}

void COMP(unsigned short source, unsigned short destination, unsigned short prpo, unsigned short w_b) {
	union shortchar result;
	result.word = regfile[0][1];
	if (w_b)
		regfile[0][destination] = ~regfile[0][destination];
	else
		regfile[0][destination] = ~result.byte[0];
}

void SWPB(unsigned short source, unsigned short destination) {
	unsigned short temp = regfile[0][destination];
	regfile[0][destination] = (temp >> 8) | (temp << 8);
}

void SXT(unsigned short source, unsigned short destination) {
	signed short temp = regfile[0][destination];
	if ((temp & 0x0080) == 0x0080) {
		regfile[0][destination] = temp | 0xFF00;
	}
}

////////////////////

void SETPRI() { //set current priority
	unsigned short new_priority = B0_2(IR);
	
	if (new_priority < word_psw.psw.current)
		word_psw.psw.current = new_priority;
	else {
		if (word_psw.psw.faulting == set) {
			//double fault
			double_fault = set;
			printf("\n[Double fault ($11)!!]");
		}
		else {
			word_psw.psw.faulting = set; //priority fault
		}

		printf("\n[Priorirty fault ($10)!! Attempted to move to higher priority. Current priority: %hx, Next priority: %hx ]\n", word_psw.psw.current, new_priority);
		word_psw.psw.faulting = unset;
	}
}

void SVC() { //control passes to address specified in vector[SA]
	unsigned short SA0 = B(IR, 0); //SA = bits 0->3
	unsigned short SA1 = B(IR, 1); //SA = bits 0->3
	unsigned short SA2 = B(IR, 2); //SA = bits 0->3
	unsigned short SA3 = B(IR, 3); //SA = bits 0->3

	unsigned short offset = Vector_offset[SA3][SA2][SA1][SA0];
	unsigned short VectorBase = 0xFFC0;
	unsigned short addr = VectorBase + offset;
	unsigned short reg_index = 0; //regfile index to store CEX and PSW in stack
	unsigned short temp_reg_val; //tmp variable that would hold the initial value of the register index used (e.g. regfile[0][index]). index = [source | destination] 

	bus(addr, &ISR_state.word, read, word); //memory.wordmem[(addr) >> 1];
	//ISR_address.word = memory.wordmem[(addr + 2) >> 1];
	bus(addr+2, &ISR_address.word, read, word);

	if (word_psw.psw.current < ISR_state.psw.current) {
		//save current state (PC, LR, word_psw.psw, CEX state)
		//ST(source, destination, prpo, w_b, dec, inc)
		ST(PC_index, SP_index, set, word, set, unset);
		ST(LR_index, SP_index, set, word, set, unset);

		temp_reg_val = regfile[0][reg_index]; //R0-register 0. Save information in register 0(initial value/data) to temp variable
		regfile[0][reg_index] = word_psw.word; //R0 gets current application psw status to store in stack
		ST(reg_index, SP_index, set, word, set, unset);

		regfile[0][reg_index] = CEX_state; //R0 gets CEX_state to stor in stack
		ST(reg_index, SP_index, set, word, set, unset);

		regfile[0][reg_index] = temp_reg_val; //R0-register 0. Recall information from temp variable to register 0 

		word_psw.word = ISR_state.word; //get psw of vector
		regfile[0][PC_index] = ISR_address.word; //get address of vector entry point for handler
		regfile[0][LR_index] = 0xFFFF; //set LR
		CEX_state = FALSE; //clear cex state information
	}
	else {
		if (word_psw.psw.faulting == set) {
			//double fault
			double_fault = set;
			printf("\n[Double fault ($11)!!]");
		}
		else {
			word_psw.psw.faulting = set; //a priority fault occurs
		}

		printf("\n[Priorirty fault ($10)!! Attempted to trap to priority of lower or same. Current priority: %hx, Next priority: %hx ]\n", word_psw.psw.current, ISR_state.psw.current);
	}

	if(!word_psw.psw.faulting){
		MOV(LR_index, PC_index);
	}
	else {
		word_psw.psw.faulting = unset;
	}
}

void SETCC() { //set word_psw.psw bits (1=set)
	int c, v, n, z, s;
	c = B(IR, 0); //carry bit
	z = B(IR,1); //zero bit
	n = B(IR, 2); //negative bit

	s = B(IR, 3); //sleep bit
	v = B(IR, 4);

	//set if set instruction is set
	word_psw.psw.c = (c == 1) ? 1 : word_psw.psw.c; 
	word_psw.psw.z = (z == 1) ? 1 : word_psw.psw.z;
	word_psw.psw.n = (n == 1) ? 1 : word_psw.psw.n;
	word_psw.psw.v = (v == 1) ? 1 : word_psw.psw.v;

	if (word_psw.psw.current == 7) { //if current priority is highest priority 7(#0111)
		word_psw.psw.slp = 0;
	}
	else {
		word_psw.psw.slp = (s == 1) ? 1 : word_psw.psw.slp;
	}
}

void CLRCC() {
	int c, v, n, z, s;
	c = B(IR, 0);
	z = B(IR, 1);
	n = B(IR, 2);
	s = B(IR, 3);
	v = B(IR, 4);

	//clear if clear instruction is set
	word_psw.psw.c = (c == 1) ? 0 : word_psw.psw.c;
	word_psw.psw.z = (z == 1) ? 0 : word_psw.psw.z;
	word_psw.psw.n = (n == 1) ? 0 : word_psw.psw.n;
	word_psw.psw.slp = (s == 1) ? 0 : word_psw.psw.slp;
	word_psw.psw.v = (v == 1) ? 0 : word_psw.psw.v;
}

void CEX() {
	unsigned short cond = B6_9(IR); //word_psw.psw condition bits
	true_count = B3_5(IR);
	false_count = B0_2(IR);

	CEX_state = TRUE; //CEX is on and is to be executed in fetch
	cond_state = FALSE;
	
	switch (cond) {
	case 0: //(EQ) equal/equals zero
		if (word_psw.psw.z == TRUE) { //if zero bit is set, condition is true
			cond_state = TRUE;
		}
		break;
	case 1: //(NE) not equal
		if (word_psw.psw.z == FALSE) { //if zero bit is clear, condition true
			cond_state = TRUE;
		}
		break;
	case 2: //(CS/HS) carry set/unsigned higher or same
		if (word_psw.psw.c == TRUE) { //if carry bit is set, condition true
			cond_state = TRUE;
		}
		break;
	case 3: //(CC/LO) carry clear/unisgned lower
		if (word_psw.psw.c == FALSE) { //if carry bit is clear, condition true
			cond_state = TRUE;
		}
		break;
	case 4: //(MI) minus/negative
		if (word_psw.psw.n == TRUE) { //if negative bit is set, condition true
			cond_state = TRUE;
		}
		break;
	case 5: //(PL) plus/positive or zero
		if (word_psw.psw.n == FALSE) { //if negative bit is clear, condition true
			cond_state = TRUE;
		}
		break;
	case 6: //(VS) overflow
		if (word_psw.psw.v == TRUE) { //if oveflow bit is set, condition true
			cond_state = TRUE;
		}
		break;
	case 7: //(VC) no overflow
		if (word_psw.psw.v == FALSE)	{ //if overflow bit is clear, condition true
			cond_state = TRUE;
		}
		break;
	case 8: //(HI) unsigned higher
		if ((word_psw.psw.c == TRUE) && (word_psw.psw.z == FALSE)) { //if carry bit is set and zero bit is clear, condition true
			cond_state = TRUE;
		}
		break;
	case 9: //(LS) unsigned lower or same
		if ((word_psw.psw.c == FALSE) && (word_psw.psw.z == TRUE)) { //if carry bit is clear and zero bit is set, condition true
			cond_state = TRUE;
		}
		break;
	case 10: //(GE) signed greater than or equal
		if (word_psw.psw.n == word_psw.psw.v) { //if negative bit equals overflow bit, condition true
			cond_state = TRUE;
		}
		break;
	case 11: //(LT) signed less than
		if (word_psw.psw.n != word_psw.psw.v)	{ //if negative bit not equal overflow bit, condition true
			cond_state = TRUE;
		}
		break;
	case 12: //(GT) signed greater than
		if ((word_psw.psw.z == FALSE) && (word_psw.psw.n == word_psw.psw.v)) { //if zero bit is clear, AND, negative and overflow bits are equal, condition true 
			cond_state = TRUE;
		}
		break;
	case 13: //(LE)* signed less than or equal
		if ((word_psw.psw.z == TRUE) || (word_psw.psw.n != word_psw.psw.v)) { //if zero bit is set, OR, negative and overflow bits are not equal, condition true
			cond_state = TRUE;
		}
		break;
	case 14: //(TR) true part is always executed
		cond_state = TRUE;
		break;
	case 15: //(FL) false part is always executed
		//cond is already set to false
		break;
	default:
		break;
	}
}

/////////////////

void LD(unsigned short source, unsigned short destination, unsigned short prpo, unsigned short w_b, unsigned short dec, unsigned short inc) {

	if (!prpo) {
		if (dec) {
			//bus(regfile[0][source], &regfile[0][destination], read, w_b);
			//access_cache_AS(regfile[0][source], &regfile[0][destination], read, w_b);//bus(MAR, MDR_addr, read, word); // pass MDR as a pointer
			switch (cache_type) {
			case ASSOCIATIVE:
				access_cache_AS(regfile[0][source], &regfile[0][destination], read, w_b);//bus(MAR, MDR_addr, read, word); // pass MDR as a pointer
				break;
			case DIRECTMAPPING:
				access_cache_DM(regfile[0][source], &regfile[0][destination], read, w_b);//bus(MAR, MDR_addr, read, word); // pass MDR as a pointer
				break;
			default:
				bus(regfile[0][source], &regfile[0][destination], read, w_b); // pass MDR as a pointer
				break;
			}
			if(w_b)
				regfile[0][source] = regfile[0][source] - 1;
			else
				regfile[0][source] = regfile[0][source] - 2;
		}
		else {
			//bus(regfile[0][source], &regfile[0][destination], read, w_b);
			//access_cache_AS(regfile[0][source], &regfile[0][destination], read, w_b);
			switch (cache_type) {
			case ASSOCIATIVE:
				access_cache_AS(regfile[0][source], &regfile[0][destination], read, w_b);//bus(MAR, MDR_addr, read, word); // pass MDR as a pointer
				break;
			case DIRECTMAPPING:
				access_cache_DM(regfile[0][source], &regfile[0][destination], read, w_b);//bus(MAR, MDR_addr, read, word); // pass MDR as a pointer
				break;
			default:
				bus(regfile[0][source], &regfile[0][destination], read, w_b); // pass MDR as a pointer
				break;
			}
			//regfile[0][source] = regfile[0][source] + 2;
			if (w_b)
				regfile[0][source] = regfile[0][source] + 1;
			else
				regfile[0][source] = regfile[0][source] + 2;
		}
	}
	else {
		if (dec) {
			//regfile[0][source] = regfile[0][source] - 2;
			if (w_b)
				regfile[0][source] = regfile[0][source] - 1;
			else
				regfile[0][source] = regfile[0][source] - 2;
			//access_cache_AS(regfile[0][source], &regfile[0][destination], read, w_b);
			switch (cache_type) {
			case ASSOCIATIVE:
				access_cache_AS(regfile[0][source], &regfile[0][destination], read, w_b);//bus(MAR, MDR_addr, read, word); // pass MDR as a pointer
				break;
			case DIRECTMAPPING:
				access_cache_DM(regfile[0][source], &regfile[0][destination], read, w_b);//bus(MAR, MDR_addr, read, word); // pass MDR as a pointer
				break;
			default:
				bus(regfile[0][source], &regfile[0][destination], read, w_b); // pass MDR as a pointer
				break;
			}
			//bus(regfile[0][source], &regfile[0][destination], read, w_b);
		}
		else {
			//regfile[0][source] = regfile[0][source] + 2;
			if (w_b)
				regfile[0][source] = regfile[0][source] + 1;
			else
				regfile[0][source] = regfile[0][source] + 2;
			//access_cache_AS(regfile[0][source], &regfile[0][destination], read, w_b);
			switch (cache_type) {
			case ASSOCIATIVE:
				access_cache_AS(regfile[0][source], &regfile[0][destination], read, w_b);//bus(MAR, MDR_addr, read, word); // pass MDR as a pointer
				break;
			case DIRECTMAPPING:
				access_cache_DM(regfile[0][source], &regfile[0][destination], read, w_b);//bus(MAR, MDR_addr, read, word); // pass MDR as a pointer
				break;
			default:
				bus(regfile[0][source], &regfile[0][destination], read, w_b); // pass MDR as a pointer
				break;
			}
			//bus(regfile[0][source], &regfile[0][destination], read, w_b);
		}
	}
}

void ST(unsigned short source, unsigned short destination, unsigned short prpo, unsigned short w_b, unsigned short dec, unsigned short inc) {

	if (!prpo) {
		if (dec) {
			//bus(regfile[0][destination], &regfile[0][source], write, w_b);
			//access_cache_AS(regfile[0][destination], &regfile[0][source], write, w_b);
			switch (cache_type) {
			case ASSOCIATIVE:
				access_cache_AS(regfile[0][destination], &regfile[0][source], write, w_b);//bus(MAR, MDR_addr, read, word); // pass MDR as a pointer
				break;
			case DIRECTMAPPING:
				access_cache_DM(regfile[0][destination], &regfile[0][source], write, w_b);//bus(MAR, MDR_addr, read, word); // pass MDR as a pointer
				break;
			default:
				bus(regfile[0][destination], &regfile[0][source], write, w_b); // pass MDR as a pointer
				break;
			}
			//regfile[0][destination] = regfile[0][destination] - 2;
			if (w_b)
				regfile[0][destination] = regfile[0][destination] - 1;
			else
				regfile[0][destination] = regfile[0][destination] - 2;
		}
		else {
			//bus(regfile[0][destination], &regfile[0][source], write, w_b);
			//access_cache_AS(regfile[0][destination], &regfile[0][source], write, w_b);
			switch (cache_type) {
			case ASSOCIATIVE:
				access_cache_AS(regfile[0][destination], &regfile[0][source], write, w_b);//bus(MAR, MDR_addr, read, word); // pass MDR as a pointer
				break;
			case DIRECTMAPPING:
				access_cache_DM(regfile[0][destination], &regfile[0][source], write, w_b);//bus(MAR, MDR_addr, read, word); // pass MDR as a pointer
				break;
			default:
				bus(regfile[0][destination], &regfile[0][source], write, w_b); // pass MDR as a pointer
				break;
			}
			//regfile[0][destination] = regfile[0][destination] + 2;
			if (w_b)
				regfile[0][destination] = regfile[0][destination] + 1;
			else
				regfile[0][destination] = regfile[0][destination] + 2;
		}
	}
	else {
		if (dec) {
			//regfile[0][destination] = regfile[0][destination] - 2;
			if (w_b)
				regfile[0][destination] = regfile[0][destination] - 1;
			else
				regfile[0][destination] = regfile[0][destination] - 2;
			//access_cache_AS(regfile[0][destination], &regfile[0][source], write, w_b);
			switch (cache_type) {
			case ASSOCIATIVE:
				access_cache_AS(regfile[0][destination], &regfile[0][source], write, w_b);//bus(MAR, MDR_addr, read, word); // pass MDR as a pointer
				break;
			case DIRECTMAPPING:
				access_cache_DM(regfile[0][destination], &regfile[0][source], write, w_b);//bus(MAR, MDR_addr, read, word); // pass MDR as a pointer
				break;
			default:
				bus(regfile[0][destination], &regfile[0][source], write, w_b); // pass MDR as a pointer
				break;
			}
			//bus(regfile[0][destination], &regfile[0][source], write, w_b);
		}
		else {
			//regfile[0][destination] = regfile[0][destination] + 2;
			if (w_b)
				regfile[0][destination] = regfile[0][destination] + 1;
			else
				regfile[0][destination] = regfile[0][destination] + 2;
			//access_cache_AS(regfile[0][destination], &regfile[0][source], write, w_b);
			switch (cache_type) {
			case ASSOCIATIVE:
				access_cache_AS(regfile[0][destination], &regfile[0][source], write, w_b);//bus(MAR, MDR_addr, read, word); // pass MDR as a pointer
				break;
			case DIRECTMAPPING:
				access_cache_DM(regfile[0][destination], &regfile[0][source], write, w_b);//bus(MAR, MDR_addr, read, word); // pass MDR as a pointer
				break;
			default:
				bus(regfile[0][destination], &regfile[0][source], write, w_b); // pass MDR as a pointer
				break;
			}
			//bus(regfile[0][destination], &regfile[0][source], write, w_b);
		}
	}
}

void MOVL(unsigned short source, signed short destination) {
	unsigned short data;

	data = B3_10(IR);
	regfile[0][destination] = (regfile[0][destination] << 8) | data;
}

void MOVLZ(unsigned short source, signed short destination) {
	unsigned short data;

	data = B3_10(IR);
	regfile[0][destination] = data & 0x00FF;
}

void MOVLS(unsigned short source, signed short destination) {
	unsigned short data;

	data = B3_10(IR);
	regfile[0][destination] = data | 0xFF00;
}

void MOVH(unsigned short source, signed short destination) {
	unsigned short data;

	data = B3_10(IR); //get data from operand
	regfile[0][destination] = (data << 8) | regfile[0][destination]; //move data to high byte of register
}

void LDR(unsigned short source, unsigned short destination, unsigned short prpo, unsigned short w_b, unsigned short dec, unsigned short inc) {
	unsigned short offset;

	offset = (IR & 0x3F80) >> 7;
	if ((offset & 0x0040) == 0x0040)
		offset |= 0xFF80;

	//regfile[0][source] = regfile[0][source] + offset;
	//access_cache_AS(regfile[0][source], &regfile[0][destination], read, w_b);
	switch (cache_type) {
	case ASSOCIATIVE:
		access_cache_AS(regfile[0][source] + offset, &regfile[0][destination], read, w_b);//bus(MAR, MDR_addr, read, word); // pass MDR as a pointer
		break;
	case DIRECTMAPPING:
		access_cache_DM(regfile[0][source] + offset, &regfile[0][destination], read, w_b);//bus(MAR, MDR_addr, read, word); // pass MDR as a pointer
		break;
	default:
		bus(regfile[0][source] + offset, &regfile[0][destination], read, w_b); // pass MDR as a pointer
		break;
	}
	//bus(regfile[0][source], &regfile[0][destination], read, w_b);
}

void STR(unsigned short source, unsigned short destination, unsigned short prpo, unsigned short w_b, unsigned short dec, unsigned short inc) {
	unsigned short offset;

	offset = (IR & 0x3F80) >> 7;
	if ((offset & 0x0040) == 0x0040)
		offset |= 0xFF80;

	
	//regfile[0][destination] = regfile[0][destination] + offset;
	w_b = B(IR, 6);

	//access_cache_AS(regfile[0][destination], &regfile[0][source], write, w_b);
	switch (cache_type) {
	case ASSOCIATIVE:
		access_cache_AS(regfile[0][destination] + offset, &regfile[0][source], write, w_b);//bus(MAR, MDR_addr, read, word); // pass MDR as a pointer
		break;
	case DIRECTMAPPING:
		access_cache_DM(regfile[0][destination] + offset, &regfile[0][source], write, w_b);//bus(MAR, MDR_addr, read, word); // pass MDR as a pointer
		break;
	default:
		bus(regfile[0][destination] + offset, &regfile[0][source], write, w_b); // pass MDR as a pointer
		break;
	}
	//bus(regfile[0][destination], &regfile[0][source], write, w_b);
}

I_set init_register_table[init_reg_sz] = {
	{"MOVL", 0x000C, MOVL}, {"MOVLZ", 0x000D, MOVLZ}, {"MOVLS", 0x000E, MOVLS},
	{"MOVH", 0x000F, MOVH}, {"LDR", 0x0002, LDR}, {"STR", 0x0003, STR}
};

I_set reg_operation_table[reg_op_sz] = {
	{"MOV", 0x0098, MOV}, {"SWAP", 0x0099, SWAP}, {"SRA", 0x09A0, SRA},
	{"RRC", 0x09A1, RRC}, {"COMP", 0x09A2, COMP}, {"SWPB", 0x09A3, SWPB},
	{"SXT", 0x09A4, SXT}, {"SETPRI", 0x04D8, SETPRI}, {"SVC", 0x04D9, SVC},
	{"SETCC", 0x09B4, SETCC}, {"CLRCC", 0x09B8, CLRCC}, {"CEX", 0x0014, CEX},
	{"LD", 0x0016, LD},	{"ST", 0x0017, ST}
};