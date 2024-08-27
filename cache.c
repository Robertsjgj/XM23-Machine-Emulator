#include "emulator.h"

CL cache[cache_sz];
CACHE_STATE state;
unsigned short ADDR = 0;

int find_value(unsigned short address) {
	unsigned short DM_addr1 = address & 0x000F; //get the last 4 bits
	int val = -1;

	for (int j = 0; j < cache_sz; j++) {
		state = miss;
		if (DM_addr1 == table[j].LSNibble) {
			val = table[j].value; //found the index where address is to be stored or read from

			if (address == cache[j].address)
				state = hit; //address requested is address found in cache line
			break;
		}
	}
	return val;
}

int init_cache() {
	memset(cache, 0, sizeof(cache));
	
	for (int i = 0; i < cache_sz; i++) {
		cache[i].empty = empty;
		cache[i].content.bytemem[cache[i].address] = 0x00;
		cache[i].dirty_bit = UNSET;
		cache[i].usage = 0;
	}
	return 1;
}

void display_cache() {
	printf("\nCache memory:\n");
	printf("line index\t Address\t Content\t Usage\t Dirty Bit\n");
	for (int i = 0; i < cache_sz; i++) {

		printf("%d\t\t0x%0hx\t\t%hx\t\t%d\t\t%d\n", i, cache[i].address, cache[i].content.wordmem[cache[i].address >> 1], cache[i].usage, cache[i].dirty_bit);
	}
}

void edit_cache_lines() {
	int line_index;
	unsigned short new_address;
	unsigned short new_word;
	unsigned char new_byte;
	int new_usage;
	enum DIRTY_BIT new_dirty_bit;
	enum EMPTY new_empty;
	int choice;

	// Ask user for cache line index
	printf("Enter the cache line index (0 to %d): ", cache_sz - 1);
	printf("ANS-> ");
	scanf_s("%d", &line_index);

	// Check the cache line index
	if (line_index < 0 || line_index >= cache_sz) {
		printf("Invalid cache line index!\n");
		return;
	}

	while (1) {
		printf("Choose a field to change: \n");
		printf("1. Address\n2. Word content\n3. Byte content\n4. Usage\n5. Dirty Bit\n6. Empty Field\n7. Quit\n");
		printf("ANS-> ");
		scanf_s("%d", &choice);

		switch (choice) {
		case 1: // Address
			printf("Enter the new address for cache line %d: ", line_index);
			printf("ANS-> ");
			scanf_s("%hx", &new_address);
			cache[line_index].address = new_address;
			break;
		case 2: // Word content
			printf("Enter the new word content for cache line %d: ", line_index);
			printf("ANS-> ");
			scanf_s("%hx", &new_word);
			cache[line_index].content.wordmem[cache[line_index].address >> 1] = new_word;
			break;
		case 3: // Byte content
			printf("Enter the new byte content for cache line %d: ", line_index);
			printf("ANS-> ");
			scanf_s("%hhu", &new_byte);
			cache[line_index].content.bytemem[0] = new_byte;
			break;
		case 4: // Usage
			printf("Enter the new usage for cache line %d: ", line_index);
			printf("ANS-> ");
			scanf_s("%d", &new_usage);
			cache[line_index].usage = new_usage;
			break;
		case 5: // Dirty bit
			printf("Enter the new dirty bit for cache line %d (0 for CLEAN, 1 for DIRTY): ", line_index);
			printf("ANS-> ");
			scanf_s("%u", (unsigned int*)&new_dirty_bit);
			cache[line_index].dirty_bit = new_dirty_bit;
			break;
		case 6: // Empty field
			printf("Enter the new empty field for cache line %d (0 for NOT_EMPTY, 1 for EMPTY): ", line_index);
			printf("ANS-> ");
			scanf_s("%u", (unsigned int*)&new_empty);
			cache[line_index].empty = new_empty;
			break;
		case 7: // Leave while loop
			break;
		default:
			printf("Invalid choice!\n");
			return;
		}
		if (choice == 7) {
			break;
		}
	}
	printf("Cache line %d updated successfully!\n", line_index);
}

void update_cache(int index, enum EMPTY empt, unsigned short addr, unsigned short read_write, unsigned short data) {
	//decrease usage of all filled cache lines
	if (cache_type == ASSOCIATIVE) {
		for (int i = 0; i < cache_sz; i++) {
			if (!cache[i].empty && cache[i].usage > cache[index].usage) {
				cache[i].usage--; //reduce usage of all other filled up cache line
			}
		}
	}
	//set state to not empty and fill address space when there is a miss
	if (!state) {
		cache[index].empty = not_empty;
		cache[index].address = addr;
	}
	if(cache_type == ASSOCIATIVE)
		cache[index].usage = cache_sz - 1; //set filled in or replaced address to newly used 
}

void access_cache_AS(unsigned short address, unsigned short *data_addr, unsigned short read_write, unsigned short word_byte) {
	int found = -1; //index holding the found index of the cache line
	int oldest_used = 0; //least frequently used cache line index

	for (int i = 0; i < cache_sz; i++) { //look if address is available in cache
		state = miss;
		if (!cache[i].empty) { //if cache is not empty
			if (address == cache[i].address) {
				state = hit; //address found
				found = i;
				break;
			}
			else if (cache[i].usage < cache[oldest_used].usage) {
				oldest_used = i; //update least used cache line
				state = miss; //address not found
			}
			else {
				state = miss; //address not found
			}
		}
 	}

	if (state) { //a hit (address is found in cache at index = found)
		cache[found].dirty_bit = SET;
		update_cache(found, not_empty, address, read, *data_addr); //update usage and address
		if (read_write == write) {
			bus_cache(found, address, &*data_addr, write, word);
		}
		else {
			cache[found].dirty_bit = UNSET;
		}
		bus_cache(found, address, &*data_addr, read, word);

		if (cache[found].dirty_bit == SET) {
			bus(address, &*data_addr, write, word_byte);
		}
	}
	else { //a miss (address not found)
		bus(address, &*data_addr, read_write, word_byte); //now we can access the primary memory. pass MDR as a pointer
		
		int empty = -1;
		for (int i = 0; i < cache_sz; i++) { //look for empty cache line
			if (cache[i].empty) {
				empty = i; //found empty space for new address
				break;
			}
		}
		if (empty != -1) { //empty cache line found
			cache[empty].dirty_bit = SET;
			if (read_write == read) {
				cache[empty].dirty_bit = UNSET;
			}
			update_cache(empty, not_empty, address, write, *data_addr);
			bus_cache(empty, address, &*data_addr, write, word);

			if (cache[empty].dirty_bit == SET) {
				bus(address, &*data_addr, write, word_byte);
			}

		}
		else { //no empty cache line found (cache is full)
			cache[oldest_used].dirty_bit = SET;
			if (read_write == read) {
				cache[oldest_used].dirty_bit = UNSET;
			}
			update_cache(oldest_used, not_empty, address, write, *data_addr);
			bus_cache(oldest_used, address, &*data_addr, write, word);

			if (cache[oldest_used].dirty_bit == SET) {
				bus(address, &*data_addr, write, word_byte);
			}
		}
	}
}

void access_cache_DM(unsigned short address, unsigned short* data_addr, unsigned short read_write, unsigned short word_byte) {
	unsigned short DM_addr = address & 0x00FF;
	int i = 0;
	
	i = find_value(address); //find index allocated for the address read

	cache[i].dirty_bit = SET;

	if (state) { //a hit (exact address is found in cache at index = found)
		cache[i].dirty_bit = SET;
		update_cache(i, not_empty, address, read, *data_addr);
		if (read_write == write) {
			bus_cache(i, address, &*data_addr, write, word);
			cache[i].usage = 0;
		}
		else {
			cache[i].dirty_bit = UNSET;
		}
		bus_cache(i, address, &*data_addr, read, word);

		if (cache[i].dirty_bit == SET) {
			bus(address, &*data_addr, write, word_byte);
		}
	}
	else { //a miss (exact address not found)
		bus(address, &*data_addr, read_write, word_byte); //now we can access the primary memory. pass MDR as a pointer

		cache[i].dirty_bit = SET;
		if (read_write == read) {
			cache[i].dirty_bit = UNSET;
		}
		else {
			cache[i].usage = 0;
		}
		update_cache(i, not_empty, address, write, *data_addr);
		bus_cache(i, address, &*data_addr, write, word);

		if (cache[i].dirty_bit == SET) {
			bus(address, &*data_addr, write, word_byte);
		}
	}
}

LookupTable table[cache_sz] = {
	{0x0, 0}, {0x1, 1}, {0x2, 2},
	{0x3, 3}, {0x4, 4}, {0x5, 5},
	{0x6, 6}, {0x7, 7}, {0x8, 8},
	{0x9, 9}, {0xA, 10}, {0xB, 11},
	{0xC, 12}, {0xD, 13}, {0xE, 14},
	{0xF, 15}
};