#pragma once

#define wordsz 1<<15 //word length
#define bytesz 1<<16 //byte length
#define cache_sz 16

//dirty bit for if cache is updated
enum DIRTY_BIT {
	UNSET, SET
};

//acces state if miss of hit
typedef enum HIT_MISS_STATE{
	miss, hit
}CACHE_STATE;
extern CACHE_STATE state;

//cache line state if empty or not
enum EMPTY{
	not_empty, empty
};

typedef enum CACHE_TYPE {
	ASSOCIATIVE, DIRECTMAPPING, NO_CACHE
}CT;
extern CT cache_type;

//cache memory union linking word and byte memory
union CACHE_MEMORY {
	unsigned short wordmem[wordsz];
	unsigned char bytemem[bytesz];
};

//cache line struct
typedef struct cache_line{
	unsigned short address;
	union CACHE_MEMORY content;
	int usage;
	enum DIRTY_BIT dirty_bit;
	enum EMPTY empty;
}CL;
extern CL cache[cache_sz]; //cache - array of cache lines


int init_cache();
extern void access_cache_AS(unsigned short address, unsigned short* data_addr, unsigned short read_write, unsigned short word_byte);
extern void access_cache_DM(unsigned short address, unsigned short* data_addr, unsigned short read_write, unsigned short word_byte);
void display_cache();
extern void edit_cache_lines();



