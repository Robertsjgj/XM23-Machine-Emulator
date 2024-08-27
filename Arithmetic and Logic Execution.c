#include "Emulator.h"

void bit_to_short(unsigned short* source_array, unsigned short* destination_array,
	unsigned short s_nib, unsigned short d_nib, int j) {
	source_array[j] = s_nib;
	destination_array[j] = d_nib;
}

void ADD(unsigned short source, signed short destination, unsigned short rc, unsigned short wb) {
	enum bool r_c;
	enum word_byte w_b;
	union shortchar s, d, result;

	r_c = rc;
	w_b = wb;

	s.word = regfile[r_c][source];
	d.word = regfile[0][destination];

	if (!w_b) {
		result.word = d.word + s.word;
		// Update PSW
		update_psw(s.word, d.word, result.word, w_b);

		regfile[0][destination] = result.word;
	}
	else {
		result.byte[0] = d.byte[0] + s.byte[0];
		// Update PSW
		update_psw(s.byte[0], d.byte[0], result.byte[0], w_b);

		regfile[0][destination] = result.byte[0];
	}
}

void ADDC(unsigned short source, signed short destination, unsigned short rc, unsigned short wb) {
	enum bool r_c;
	enum word_byte w_b;
	union shortchar s, d, result;

	r_c = rc;
	w_b = wb;

	s.word = regfile[r_c][source];
	d.word = regfile[0][destination];

	if (!w_b) {
		result.word = d.word + s.word + word_psw.psw.c;
		update_psw(s.word + word_psw.psw.c, d.word, result.word, w_b);
		regfile[0][destination] = result.word;
	}
	else {
		result.byte[0] = d.byte[0] + s.byte[0] + word_psw.psw.c;
		update_psw(s.byte[0] + word_psw.psw.c, d.byte[0], result.byte[0], w_b);
		regfile[0][destination] = result.byte[0];
	}
}

void SUB(unsigned short source, signed short destination, unsigned short rc, unsigned short wb) {
	enum bool r_c;
	enum word_byte w_b;
	union shortchar s, d, result;

	r_c = rc;
	w_b = wb;

	s.word = regfile[r_c][source];
	d.word = regfile[0][destination];

	if (!w_b) {
		result.word = d.word + ~s.word + 1;
		regfile[0][destination] = result.word;

		update_psw(~s.word + 1, d.word, result.word, w_b);
	}
	else {
		result.byte[0] = d.byte[0] + ~s.byte[0] + 1;
		regfile[0][destination] = result.byte[0];

		update_psw(~s.byte[0] + 1, d.byte[0], result.byte[0], w_b);
	}
}

void SUBC(unsigned short source, signed short destination, unsigned short rc, unsigned short wb) {
	enum bool r_c;
	enum word_byte w_b;
	union shortchar s, d, result;

	r_c = rc;
	w_b = wb;

	s.word = regfile[r_c][source];
	d.word = regfile[0][destination];

	if (!w_b) {
		result.word = d.word + ~s.word + word_psw.psw.c;
		update_psw(~s.word + word_psw.psw.c, d.word, result.word, w_b);
		regfile[0][destination] = result.word;
	}
	else {
		result.byte[0] = d.byte[0] - ~s.byte[0] + word_psw.psw.c;
		update_psw(~s.byte[0] + word_psw.psw.c, d.byte[0], result.byte[0], w_b);
		regfile[0][destination] = result.byte[0];
	}
}



void DADD(unsigned short source, signed short destination, unsigned short rc, unsigned short wb) {
	enum bool r_c;
	enum word_byte w_b;
	union BCD_NUM s, d, sum;
	unsigned short carry = 0, HC = word_psw.psw.c;

	r_c = rc;
	w_b = wb;

	d.word = regfile[0][destination];
	s.word = regfile[r_c][source];

	unsigned short sum_n[4], s_n[4], d_n[4];
	int j = 0;
	bit_to_short(&s_n, &d_n, s.NIB.N0, d.NIB.N0, j);
	bit_to_short(&s_n, &d_n, s.NIB.N1, d.NIB.N1, j + 1);
	bit_to_short(&s_n, &d_n, s.NIB.N2, d.NIB.N2, j + 2);
	bit_to_short(&s_n, &d_n, s.NIB.N3, d.NIB.N3, j + 3);

	for (int i = 0; i < 4; i++) {
		sum_n[i] = s_n[i] + d_n[i] + HC;
		HC = (sum_n[i] > 9) ? 1 : 0; // set half carry if sum nibble is greater than 9
		sum_n[i] = sum_n[i] - 10 * HC;
	}

	int i = 0;
	sum.NIB.N0 = sum_n[i];
	sum.NIB.N1 = sum_n[i + 1];
	sum.NIB.N2 = sum_n[i + 2];
	sum.NIB.N3 = sum_n[i + 3];

	/*//add the individual nibbles and half carry
	sum.NIB.N0 = s.NIB.N0 + d.NIB.N0 + HC;
	HC = (sum.NIB.N0 > 9) ? 1 : 0; //set half carry if sum nibble is greater than 9
	sum.NIB.N0 = HC ? sum.NIB.N0 - 10 : sum.NIB.N0;

	sum.NIB.N1 = s.NIB.N1 + d.NIB.N1 + HC;
	HC = (sum.NIB.N1 > 9) ? 1 : 0; //set half carry is sum nibble is greater than 9
	sum.NIB.N1 = HC ? sum.NIB.N1 - 10 : sum.NIB.N1;

	sum.NIB.N2 = s.NIB.N2 + d.NIB.N2 + HC;
	HC = (sum.NIB.N2 > 9) ? 1 : 0; //set half carry if sum nibble is greater than 9
	sum.NIB.N2 = HC ? sum.NIB.N2 - 10 : sum.NIB.N2;

	sum.NIB.N3 = s.NIB.N3 + d.NIB.N3 + HC;
	HC = (sum.NIB.N3 > 9) ? 1 : 0; //set half carry if sum nibble is greater than 9
	sum.NIB.N3 = HC ? sum.NIB.N3 - 10 : sum.NIB.N3;*/

	//put sum in the destination register
	regfile[0][destination] = (wb) ? sum.word : sum.word & 0x00FF;
}

void CMP(unsigned short source, signed short destination, unsigned short rc, unsigned short wb) {
	enum bool r_c;
	enum word_byte w_b;
	union shortchar s, d, result;

	r_c = rc;
	w_b = wb;

	s.word = regfile[r_c][source];
	d.word = regfile[0][destination];

	if (!w_b) {
		result.word = d.word + ~s.word + 1;

		update_psw(~s.word + 1, d.word, result.word, w_b);
	}
	else {
		result.byte[0] = d.byte[0] + ~s.byte[0] + 1;

		update_psw(~s.byte[0] + 1, d.byte[0], result.byte[0], w_b);
	}
}

void XOR(unsigned short source, signed short destination, unsigned short rc, unsigned short wb) {
	enum bool r_c;
	enum word_byte w_b;
	union shortchar s, d, result;

	r_c = rc;
	w_b = wb;

	s.word = regfile[r_c][source];
	d.word = regfile[0][destination];

	if (!w_b) {
		result.word = d.word ^ s.word;
		update_psw(s.word, d.word, result.word, w_b);
		regfile[0][destination] = result.word;
	}
	else {
		result.byte[0] = d.byte[0] ^ s.byte[0];
		update_psw(s.byte[0], d.byte[0], result.byte[0], w_b);
		regfile[0][destination] = result.byte[0];
	}
}

void OR(unsigned short source, signed short destination, unsigned short rc, unsigned short wb) {
	enum bool r_c;
	enum word_byte w_b;
	union shortchar s, d, result;

	r_c = rc;
	w_b = wb;

	s.word = regfile[r_c][source];
	d.word = regfile[0][destination];

	if (!w_b) {
		result.word = d.word | s.word;
		update_psw(s.word, d.word, result.word, w_b);
		regfile[0][destination] = result.word;
	}
	else {
		result.byte[0] = d.byte[0] | s.byte[0];
		update_psw(s.byte[0], d.byte[0], result.byte[0], w_b);
		regfile[0][destination] = result.byte[0];
	}
}

void AND(unsigned short source, signed short destination, unsigned short rc, unsigned short wb) {
	enum bool r_c;
	enum word_byte w_b;
	union shortchar s, d, result;

	r_c = rc;
	w_b = wb;

	s.word = regfile[r_c][source];
	d.word = regfile[0][destination];

	if (!w_b) {
		result.word = d.word & s.word;
		update_psw(s.word, d.word, result.word, w_b);
		regfile[0][destination] = result.word;
	}
	else {
		result.byte[0] = d.byte[0] & s.byte[0];
		update_psw(s.byte[0], d.byte[0], result.byte[0], w_b);
		regfile[0][destination] = result.byte[0];
	}
}

void BIT(unsigned short source, signed short destination, unsigned short rc, unsigned short wb) {
	enum bool r_c;
	enum word_byte w_b;
	union shortchar s, d, result;

	r_c = rc;
	w_b = wb;

	s.word = regfile[r_c][source];
	d.word = regfile[0][destination];

	if (!w_b) {
		result.word = d.word & (1 << s.word);
		update_psw(s.word, d.word, result.word, w_b);
	}
	else {
		result.byte[0] = d.byte[0] & (1 << s.byte[0]);
		update_psw(s.byte[0], d.byte[0], result.byte[0], w_b);
	}
}

void BIC(unsigned short source, signed short destination, unsigned short rc, unsigned short wb) {
	enum bool r_c;
	enum word_byte w_b;
	union shortchar s, d, result;

	r_c = rc;
	w_b = wb;

	s.word = regfile[r_c][source];
	d.word = regfile[0][destination];

	if (!w_b) {
		result.word = d.word & ~(1 << s.word);
		update_psw(s.word, d.word, result.word, w_b);
		regfile[0][destination] = result.word;
	}
	else {
		result.byte[0] = d.byte[0] & ~(1 << s.byte[0]);
		update_psw(s.byte[0], d.byte[0], result.byte[0], w_b);
		regfile[0][destination] = result.byte[0];
	}
}

void BIS(unsigned short source, signed short destination, unsigned short rc, unsigned short wb) {
	enum bool r_c;
	enum word_byte w_b;
	union shortchar s, d, result;

	r_c = rc;
	w_b = wb;

	s.word = regfile[r_c][source];
	d.word = regfile[0][destination];

	if (!w_b) {
		result.word = d.word | (1 << s.word);
		update_psw(s.word, d.word, result.word, w_b);
		regfile[0][destination] = result.word;
	}
	else {
		result.byte[0] = d.byte[0] | (1 << s.byte[0]);
		update_psw(s.byte[0], d.byte[0], result.byte[0], w_b);
		regfile[0][destination] = result.byte[0];
	}
}

I_set arithmetic_table[arith_sz] = {
	{"ADD", 0x0040, ADD}, {"ADDC", 0x0041, ADDC}, {"SUB", 0x0042, SUB},
	{"SUBC", 0x0043, SUBC}, {"DADD", 0x0044, DADD}, {"CMP", 0x0045, CMP},
	{"XOR", 0x0046, XOR}, {"AND", 0x0047, AND}, {"OR", 0x0048, OR},
	{"BIT", 0x0049, BIT}, {"BIC", 0x004A, BIC}, {"BIS", 0x004B, BIS}
};