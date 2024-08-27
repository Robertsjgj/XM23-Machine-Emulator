#include "Emulator.h"

unsigned short regfile[2][rfsz] = { {0, 0, 0, 0, 0, 0, 0xFFC0, 0}, {0, 1, 2, 4, 8, 16, 32, -1} }; //register file from register 0 to 7
unsigned short IR = 0x00; //instruction register

unsigned short table_index = 0; //index of instruction table to execute instruction
enum type_instr instruction_type = 0;
enum bool CEX_state = FALSE; //CEX is off as not CEX innstruction fetched

unsigned short opcode = 0x00, opcode_4h = 8900, opcode_4l = 0, breakpoint = 0x00; //opcode_4l - 4 MSBits of opcode, opcode_3l - 4 LSBits of opcode
//offset, pre/post, decrement, increment, register/constant, read/write, word/byte, source, destination
signed short offset = 0x00, prpo = 0, dec = 0, inc = 0, rc = 0, rw = 0, wb = 0, sc = 0, d = 0;

int Clock = 0; //system clock

//change the value of pc
void change_pc() {
	printf("Enter new PC value: ");
	scanf_s("%hx", &PC);
}

//fetch and instructions and data
void fetch(unsigned short* MDR_addr) {
	unsigned short MAR = 0x00;

	unsigned short reg_index = 0; //regfile index to store CEX and PSW in stack
	unsigned short temp_reg_val; //tmp variable that would hold the initial value of the register index used (e.g. regfile[0][index]). index = [source | destination] 

	MAR = PC;
	if (MAR == 0xFFFF) {
		//restore current state (PC, LR, word_psw.psw, CEX state)
		//ST(source, destination, prpo, w_b, dec, inc)
		

		temp_reg_val = regfile[0][reg_index]; //R0-register 0. Save information in register 0(initial value/data) to temp variable

		LD(SP_index, reg_index, unset, word, unset, set); //get psw state from stack into R0
		CEX_state = regfile[0][reg_index]; //get psw state from R0
		LD(SP_index, reg_index, unset, word, unset, set); //get CEX_state from stack into R0
		word_psw.word = regfile[0][reg_index]; //get CEX_state from R0
		LD(SP_index, LR_index, unset, word, unset, set);
		
		LD(SP_index, PC_index, unset, word, unset, set);
		MAR = PC;
				
		regfile[0][reg_index] = temp_reg_val; //R0-register 0. Recall information from temp variable to register 0 

	}
	if (MAR & 0x1) { //if MAR is an odd address
		if (word_psw.psw.faulting == set) {
			//double fault
			double_fault = set;
			printf("\n[Double fault ($11)!!]");
		}
		else {
			word_psw.psw.faulting = set; //invalid address fault
		}
		printf("\n[Invalid address ($9)!! Odd number address in PC. PC(address) = 0x%04x\n", PC);
		word_psw.psw.faulting = unset;
	}

	CR = set;

	if (CR) {
		switch (cache_type) {
		case ASSOCIATIVE:
			access_cache_AS(MAR, MDR_addr, read, word);//bus(MAR, MDR_addr, read, word); // pass MDR as a pointer
			break;
		case DIRECTMAPPING:
			access_cache_DM(MAR, MDR_addr, read, word);//bus(MAR, MDR_addr, read, word); // pass MDR as a pointer
			break;
		default:
			bus(MAR, MDR_addr, read, word); // pass MDR as a pointer
			break;
		}
		PC += 2;
		IR = *MDR_addr; // assign the value at address MDR to IR
	}
	Clock += 1;
}

//deocde the fetched instruction to get instruction from ISA
void decode() {
	unsigned short pc = PC;
	if (!IR) {
		if (word_psw.psw.faulting == set) {
			//double fault
			double_fault = set;
			printf("\n[Double fault ($11)!!]");
		}
		else {
			word_psw.psw.faulting = set; //illegal instruction fault
		}

		printf("\n[Illegal instruction ($8)!! Instruction register is empty: 0x%04x at address: 0x%04x]\n", IR, pc - 2);
		word_psw.psw.faulting = unset;
	}
	else {
		opcode = B13_15(IR); //get 3 MSBits

		switch (opcode) {
		case 0://branch with link
			branch_link_decode(opcode, &offset); //instruction BL
			instruction_type = branch_link;
			break;
		case 1://branch with condition
			branch_with_conditions_decode(opcode, &offset); //from instruction BEQ to BRA
			instruction_type = branch_condition;
			break;
		case 2://arithmetic operations
			opcode_4h = B12_15(IR); //get high byte of the 8 MSBits of opcode
			opcode_4l = B8_11(IR); //get lower byte of the 8 MSBits of opcode

			if (opcode_4h == 4 && opcode_4l < 12) {
				arithmetic_decode(opcode, &rc, &wb, &sc, &d);
				instruction_type = arithmetic;
			}
			else {
				reg_operation_decode(opcode, &prpo, &dec, &inc, &wb, &sc, &d);
				instruction_type = reg_operation;
			}
			break;
		case 3:
			init_register_decode(opcode, &prpo, &dec, &inc, &wb, &sc, &d); //from instruction MOVL to MOVH
			instruction_type = init_reg;
			break;
		default:
			init_register_decode(opcode, &prpo, &dec, &inc, &wb, &sc, &d);
			instruction_type = init_reg;
			break;
		}
	}
	Clock += 1;
}

//execute the decoded instruction
void execute() {
	switch (instruction_type)
	{
	case branch_link:
		branch_table[table_index].function(opcode, offset);
		break;
	case branch_condition:
		branch_table[table_index].function(opcode, offset);
		break;
	case arithmetic:
		arithmetic_table[table_index].function(sc, d, rc, wb);
		break;
	case init_reg:
		init_register_table[table_index].function(sc, d, prpo, wb, dec, inc);
		break;
	case reg_operation:
		reg_operation_table[table_index].function(sc, d, prpo, wb, dec, inc);
		break;
	default:
		break;
	}
	Clock += 1;
}

//run the cpu functions, fetch decode and execute
void cpu() {
	unsigned short MDR = 0x00, MDR_prev = 0x00;
	int cpu_go;
	cpu_go = TRUE;

	Clock -= Clock;
	printf("Start PC: 0x%04x\t", PC);
	while (cpu_go)
	{
		if (!CEX_state) { //if CEX state is OFF/FALSE. CEX instruction invalid or not read
			fetch(&MDR);
			decode();
			execute();
			if (double_fault) {
				cpu_go = 0;
			}
		}
		else { //CEX instruction is ON/TRUE
			while (true_count != 0) {
				if (!cond_state) //if condition is not met - Cond is FALSE. Do not execute true instructions
					fetch(&MDR);
				else { //is cond is TRUE. Execute true instructions
					fetch(&MDR);
					decode();
					execute();
				}
				true_count--;
			}
			while (false_count != 0) {
				if (cond_state) //if condition is met - Cond is TRUE. Do not execute false instructions
					fetch(&MDR);
				else { //is cond FALSE. Execute false instructions
					fetch(&MDR);
					decode();
					execute();
				}
				false_count--;
			}
			
			if (true_count == 0 && false_count == 0)
				CEX_state = FALSE;
			else
				printf("\nINVALID CEX EXECUTION!!\n");
		}
		/* Continue? */
		if (breakpoint)
			cpu_go = !ctrl_c_fnd && PC != breakpoint;
		else
			cpu_go = MDR - MDR_prev;

		MDR_prev = MDR;

		if (double_fault)
			cpu_go = 0;
	}
	printf("System Clock: %d\t", Clock);
	if (breakpoint)
		printf("Breakpoint: % hx\n", breakpoint);
	else
		printf("\n");
}

void break_point() {
	//end address to carry out instructions to
	printf("Enter breakpoint: ");
	scanf_s("%hx", &breakpoint);
}