#include <signal.h> /* Signal handling software */
#include "cache.h"
#define arith_sz 12 //size of arithmetic table
#define branch_sz 9 //size of branch table
#define init_reg_sz 6 //size of regiter initialization table
#define reg_op_sz 14 //size of register opertion table
#define init_new_priority 0

typedef void (*Instruction_Function)(unsigned short); //define function type and arguments for table

typedef struct Instruction {
    char* name;           //name of the instruction (e.g., "ADD", "SUB")
    unsigned short opcode; //opcode for the instruction (e.g., 0x0001, 0x0029)
    Instruction_Function function; //function to execute intruction
} I_set;

typedef struct {
    unsigned short LSNibble; //least significant nibble
    int value;
} LookupTable;

//set tyoe of instruction to execute
enum type_instr {
    branch_link, branch_condition, arithmetic, init_reg, reg_operation
};

//volatile to not cache or optimize
extern volatile sig_atomic_t ctrl_c_fnd; // T/F - whether ^C detected or not

extern unsigned short table_index; //index of instruction table to execute instruction
extern enum type_instr instruction_type; //declare intruction type
extern int Clock; //declare system clock
extern enum bool CEX_state, cond_state;
extern unsigned short true_count;
extern unsigned short false_count;
extern int double_fault;

//declare external functions to be acessible in other file//
extern void break_point();
extern void decode();
extern void arithmetic_decode(unsigned short opcode, unsigned short* rc, unsigned short* wb, unsigned short* sc, unsigned short* d);
extern void branch_with_conditions_decode(unsigned short opcode, unsigned short* offset);
extern void init_register_decode(unsigned short opcode, unsigned short* prpo, unsigned short* dec, unsigned short* inc, unsigned short* wb, unsigned short* sc, unsigned short* d);
extern void reg_operation_decode(unsigned short opcode, unsigned short* prpo, unsigned short* dec, unsigned short* inc, unsigned short* wb, unsigned short* sc, unsigned short* d);


extern void BL(unsigned short opcode, signed short offset);
extern void BEQ(unsigned short opcode, signed short offset);
extern void BRA(unsigned short opcode, signed short offset);
extern void BLT(unsigned short opcode, signed short offset);
extern void BNE(unsigned short opcode, signed short offset);
extern void BC(unsigned short opcode, signed short offset);
extern void BNC(unsigned short opcode, signed short offset);
extern void BN(unsigned short opcode, signed short offset);
extern void BGE(unsigned short opcode, signed short offset);

extern void ADD(unsigned short source, signed short destination, unsigned short rc, unsigned short wb);
extern void ADDC(unsigned short source, signed short destination, unsigned short rc, unsigned short wb);
extern void SUB(unsigned short source, signed short destination, unsigned short rc, unsigned short wb);
extern void SUBC(unsigned short source, signed short destination, unsigned short rc, unsigned short wb);
extern void DADD(unsigned short source, signed short destination, unsigned short rc, unsigned short wb);
extern void CMP(unsigned short source, signed short destination, unsigned short rc, unsigned short wb);
extern void XOR(unsigned short source, signed short destination, unsigned short rc, unsigned short wb);
extern void OR(unsigned short source, signed short destination, unsigned short rc, unsigned short wb);
extern void AND(unsigned short source, signed short destination, unsigned short rc, unsigned short wb);
extern void BIT(unsigned short source, signed short destination, unsigned short rc, unsigned short wb);
extern void BIC(unsigned short source, signed short destination, unsigned short rc, unsigned short wb);
extern void BIS(unsigned short source, signed short destination, unsigned short rc, unsigned short wb);

extern void LD(unsigned short source, unsigned short destination, unsigned short prpo, unsigned short w_b, unsigned short dec, unsigned short inc);
extern void ST(unsigned short source, unsigned short destination, unsigned short prpo, unsigned short w_b, unsigned short dec, unsigned short inc);
extern void MOVL(unsigned short source, signed short destination);
extern void MOVLZ(unsigned short source, signed short destination);
extern void MOVLS(unsigned short source, signed short destination);
extern void MOVH(unsigned short source, signed short destination);
extern void LDR(unsigned short source, unsigned short destination, unsigned short prpo, unsigned short w_b, unsigned short dec, unsigned short inc);
extern void STR(unsigned short source, unsigned short destination, unsigned short prpo, unsigned short w_b, unsigned short dec, unsigned short inc);

//declare tables to be used and accessed in other files
extern I_set branch_table[branch_sz];
extern I_set arithmetic_table[arith_sz];
extern I_set init_register_table[init_reg_sz];
extern I_set reg_operation_table[reg_op_sz];
extern LookupTable table[cache_sz];
