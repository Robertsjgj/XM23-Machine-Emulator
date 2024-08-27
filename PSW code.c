#include "Emulator.h"

struct psw_bits psw;
union psw_union word_psw;
unsigned carry[2][2][2] = { 0, 0, 1, 0, 1, 0, 1, 1 };
unsigned overflow[2][2][2] = { 0, 1, 0, 0, 0, 0, 1, 0 };

void update_psw(unsigned short src, unsigned short dst, unsigned short res, unsigned short wb)
{
	/*
	 - Update the PSW bits (V, N, Z, C)
	 - Using src, dst, and res values and whether word or byte
	 - ADD, ADDC, SUB, and SUBC
	*/
	unsigned short mss, msd, msr; /* Most significant src, dst, and res bits */

	if (wb == 0)
	{
		mss = B(src, 15);
		msd = B(dst, 15);
		msr = B(res, 15);
	}
	else /* Byte */
	{
		mss = B(src, 7);
		msd = B(dst, 7);
		msr = B(res, 7);
		res &= 0x00FF;	/* Mask high byte for 'z' check */
	}

	/* Carry */
	word_psw.psw.c = carry[mss][msd][msr];
	/* Zero */
	word_psw.psw.z = (res == 0);
	/* Negative */
	word_psw.psw.n = (msr == 1);
	/* oVerflow */
	word_psw.psw.v = overflow[mss][msd][msr];
}
