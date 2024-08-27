#include "Emulator.h"

#define pos_data 8

unsigned int len = 0, ah = 0, al = 0, data_b = 0;
unsigned int nibble = 0, last_byte = 0;
unsigned short address = 0x00;
unsigned char checksum = 0;
char type = 0;
char* path = 0;

extern union MEMORY memory;
//F_STATE state;

//write data to memory
void mem_write(unsigned short address, unsigned int data) {
    bus(address, &data, write, byte);
}

void s0_rec() {
    //read in the first three bytes for the length and address
    sscanf_s(&inrec[2], "%2x%2x%2x", &len, &ah, &al);

    //get the 16 bit address to be used in memory
    address = ah << 8 | al;

    //add the length, and address to the checksum
    checksum += len;
    checksum += ah;
    checksum += al;

    //take out the two address bytes(address high and address low)
    len -= 2;

    //
    nibble = pos_data;

    printf("Name of input file: ");

    //read in the data from the s-record
    while (len > 1) {
        sscanf_s(&inrec[nibble], "%2x", &data_b); //data_b = data store per byte

        printf("%c", data_b);
        len = len - 1;
        nibble += 2;
        checksum += data_b;
    }
    printf("\n\n");
}

void s1_rec() {
    sscanf_s(&inrec[2], "%2x%2x%2x", &len, &ah, &al);

    address = ah << 8 | al;

    checksum += len;
    checksum += ah;
    checksum += al;

    len -= 2;
    nibble = pos_data;

    while (len > 1) {
        sscanf_s(&inrec[nibble], "%2x", &data_b); //data_b = data store per byte

        //write read data to memory
        mem_write(address, data_b);

        address += 1;

        len = len - 1;
        nibble += 2; //byte read
        checksum += data_b;
    }
}

void s9_rec() {
    sscanf_s(&inrec[2], "%2x%2x%2x", &len, &ah, &al);

    address = ah << 8 | al;
    PC = address;

    checksum += len;
    checksum += ah;
    checksum += al;

    nibble = pos_data;

    //printf("START ADDRESS!!\n");
    //printf("hex       dec\n");
    //printf("%4x     %d\n\n\n", address, address);
}

void load_file() {
    char filename[256];  // Adjust the size as needed

    printf("Please enter the file name: ");
    fgets(filename, sizeof(filename), stdin);

    // Remove trailing newline character, if there is one
    if ((strlen(filename) > 0) && (filename[strlen(filename) - 1] == '\n'))
        filename[strlen(filename) - 1] = '\0';

    if (fopen_s(&outfile, "data.txt", "w") != NULL) {
        printf("Error opening file.\n");
        return 1;
    }

    if (fopen_s(&infile, filename, "r") != 0)
    {
        printf("Error opening file >%s< - possibly missing\n", filename);
        getchar();
        return -1;
    }

    printf("Opened file: %s\n\n\n", filename);
}

void loader() {

    load_file();

    while (fgets(inrec, MXN, infile))
    {
        if (inrec[0] != 'S') {
            printf("Invalid s%c-record!! Cannot compute the record:\n", inrec[1]);
        }
        else {
            //read in and distinquish the S#
            type = inrec[1];
            switch (type) {
            case '0':
                s0_rec();
                break;
            case '1':
                s1_rec();
                break;
            case '9':
                s9_rec();
                break;
            default:
                printf("Invalid s-record!! S-record not within the S0, S1 and S9 possible records.\n");
                break;
            }

            sscanf_s(&inrec[nibble], "%2x", &last_byte);

            checksum += last_byte; //and with 0Xff

            if (checksum != 0xff)
                printf("S-record is invalid: %s\n", inrec);

            checksum = 0;
        }
    }

    fclose(infile);
    fclose(outfile);
}