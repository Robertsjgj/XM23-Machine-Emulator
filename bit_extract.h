#define B(x, y)	(((x)>>y) & 0x0001) //extract bit y from x 16bit storage
#define B14_15(x)	(((x)>>14) & 0x0003) //extract bits 14 to 15
#define B13_15(x)	(((x)>>13) & 0x0007) //extract bits 13 to 15
#define B11_15(x)	(((x)>>11) & 0x001F) //extract bits 11 to 15
#define B10_15(x)	(((x)>>10) & 0x003F) //extract bits 10 to 15
#define B8_15(x)	(((x)>>8) & 0x00FF) //extract bits 8 to 15
#define B7_15(x)	(((x)>>7) & 0x01FF) //extract bits 7 to 15
#define B3_15_mod(x)	(((x)>>3) & 0x1FF7) //extract bits 3 to 15. making the word_byte bit zero for simplicity
#define B3_15_mod2(x)	(((x)>>3) & 0x1FFC) //extract bits 3 to 15. making the overflow(V) and SLEEP(SLP) bits zero for simplicity
#define B4_15(x)	(((x)>>4) & 0x0FFF) //extract bits 3 to 15. making the word_byte bit zero for simplicity
#define B6_15(x)	(((x)>>6) & 0x03FF)
#define B5_15(x)	(((x)>>5) & 0x07FF)
#define B12_15(x) ((x & 0xF000) >> 12) //high 4 MSBits of opcode
#define B8_11(x) ((x & 0x0F00) >> 8) //low four LSbits of opcode
#define B3_10(x) ((x & 0x07F8) >> 3) //byte data bits from bit 3 to 10 for initializing instructions like MOV
#define B0_3(x) (x & 0x000F)
//destination bits in a register
#define B0_2(x) (x & 0x0007)
//source or constant bits in a register
#define B3_5(x) ((x & 0x0038) >> 3)

#define B6_9(x) ((x>>6) & 0x000F)

#define offsetB0_12(x)	((x) & 0x1FFF) //offset for branch with link
#define offsetB0_9(x)	((x) & 0x03FF) //offset for branch with condition