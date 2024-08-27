#pragma once
#include <stdio.h>
#include <string.h>
#include "CPU.h"
#include "bit_extract.h"
#include "cache.h"

#define MXN 256 //array size for string
#define rfsz 8 //register file size
#define LR regfile[0][5] //link register
#define SP regfile[0][6] //stack pointer
#define PC regfile[0][7] //program counter
#define SIZE 64*1024 //memory size 64KiB

#define PC_index 7
#define LR_index 5
#define SP_index 6

FILE* infile;
FILE* outfile;
char inrec[MXN];

#define wordsz 1<<15 //word length
#define bytesz 1<<16 //byte length

//memory union linking word and byte memory
union MEMORY {
	unsigned short wordmem[wordsz];
	unsigned char bytemem[bytesz];
};

//different memory for arithmetic operations
union shortchar
{
	unsigned short word;
	unsigned char byte[2];
};

enum bool{
	FALSE, TRUE
};

enum control_regiter {
	unset, set
}CR;

enum read_write {
	read, write
};

enum word_byte {
	word, byte
};

//nibble struct
struct NIBBLES {
	unsigned short N0 : 4; //1st nibble
	unsigned short N1 : 4; //2nd nibble
	unsigned short N2 : 4; //3rd nibble
	unsigned short N3 : 4; //4th nibble
};

//declaring the PSW structure
struct psw_bits
{
	unsigned short c : 1;	//carry
	unsigned short z : 1;	//zero  
	unsigned short n : 1;   //negative 
	unsigned short slp : 1; //sleep rate  
	unsigned short v : 1;	//overflow
	unsigned short current : 3;		//current priority
	unsigned short faulting : 1;	//0 - No fault, 1 - Active fault
	unsigned short reserved : 4;
	unsigned short previous : 3;	//previous priority
};
extern struct psw_bits psw;

union psw_union {
	struct psw_bits psw;
	unsigned short word;
};
extern union psw_union word_psw;
extern union psw_union ISR_state;
extern union psw_union ISR_address;

extern unsigned carry[2][2][2]; //carry table declared
extern unsigned overflow[2][2][2]; //overflow table declared
extern unsigned short regfile[2][8]; //register file table declared

//BCD operations
union BCD_NUM {
	unsigned short word;
	struct NIBBLES NIB; //nibbles
};

extern unsigned short IR; //instruction register carrying the current instruction

extern void cpu();
extern void bus(unsigned short MAR, unsigned short* MDR, unsigned short read_write, unsigned short word_byte);
extern void bus_cache(unsigned short index, unsigned short MAR, unsigned short* MDR, unsigned short read_write, unsigned short word_byte);
extern void loader();
extern void debugger();
extern void display_memory();
extern void change_pc();
extern void update_psw(unsigned short src, unsigned short dst, unsigned short res, unsigned short wb);