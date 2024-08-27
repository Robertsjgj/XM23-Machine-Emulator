#include "Emulator.h"

union MEMORY memory;

void bus(unsigned short MAR, unsigned short* data_addr, unsigned short read_write, unsigned short word_byte) {

    if (read_write == read) {
        if (word_byte == word)
            *data_addr = memory.wordmem[MAR >> 1]; // dereference MDR to assign value
        else
            *data_addr = memory.bytemem[MAR]; // dereference MDR to assign value

    }
    else {
        if (word_byte == 0)
            memory.wordmem[MAR >> 1] = *data_addr; // use the value at address MDR
        else
            memory.bytemem[MAR] = *data_addr & 0xFF; // use the value at address MDR
    }
    Clock += 3; //increase clock by 3 for memory access
}

void bus_cache(unsigned short index, unsigned short MAR, unsigned short* data_addr, unsigned short read_write, unsigned short word_byte) {
    unsigned short prev_data;

    if (read_write == read) {
        if (word_byte == word)
            *data_addr = cache[index].content.wordmem[cache[index].address>>1]; // dereference MDR to assign value
        else
            *data_addr = cache[index].content.bytemem[cache[index].address]; // dereference MDR to assign value

    }
    else {
        /*if (cache[index].dirty_bit = SET && state == hit) {
            if (word_byte == 0)
                prev_data = cache[index].content.wordmem[cache[index].address >> 1];
            else
                prev_data = cache[index].content.bytemem[cache[index].address];
        }*/

        if (word_byte == 0)
            cache[index].content.wordmem[cache[index].address >> 1] = *data_addr; // use the value at address MDR
        else
            cache[index].content.bytemem[cache[index].address] = *data_addr & 0xFF; // use the value at address MDR
    
        /*if (cache[index].dirty_bit = SET && state == hit) {
            if(word_byte == word && prev_data != cache[index].content.wordmem[cache[index].address >> 1])
                cache[index].dirty_bit = SET;
            else if (word_byte == byte && prev_data != cache[index].content.bytemem[cache[index].address]) {
                cache[index].dirty_bit = SET;
            }
            else {
                cache[index].dirty_bit = UNSET;
            }
        }*/
        
    }
}

void display_memory() {
    unsigned short start, stop;
    unsigned int i;

    //ask user for start and stop address
    printf("Enter the start and stop address in hex (eg start stop: 0000 1000): ");
    scanf_s("%4hx %4hx", &start, &stop);

    unsigned short interval_start, interval_end;

    // loop over intervals of 
    for (interval_start = start; interval_start < stop; interval_start = interval_end) {
        interval_end = interval_start + 0x10;

        // print interval
        printf("\nInterval: %4x - %4x\n", interval_start, interval_end);

        // print out data from memory within the interval
        for (i = interval_start; i < interval_end && i < stop; i++) {
            unsigned char mem_val = memory.bytemem[i];
            unsigned char mem_val2 = memory.bytemem[i+1];

            printf(" %02x %02x", mem_val2, mem_val);

            // after every 16th byte (or 8 memory values), print a newline
            if ((i + 1) % 16 == 0) {
                printf("\n");
            }
            i += 1;
        }

        // print newline if there isn't one already
        if (i % 16 != 0) {
            printf("\n");
        }
        printf("\n");
    }
}


