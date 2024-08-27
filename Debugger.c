#include "Emulator.h"
union MEMORY memory;
CT cache_type;

void display_CPU_registers() {
    int i = 0;

    printf("Name\tValue\tDecimal\tCharacter\n");
    for (i; i < rfsz; i++) { //make character c a period if number in regfile not found in ascii table. for non printable numbers
        char c = (regfile[0][i] >= 32 && regfile[0][i] <= 126) ? regfile[0][i] : '.';
        printf("R%d \t%04x\t%d\t%c\n", i, regfile[0][i], regfile[0][i], c);
    }
}

void display_psw_bits() {
    printf("PSW BITS!!\n");
    printf("Carry: %d\n", word_psw.psw.c);
    printf("Zero: %d\n", word_psw.psw.z);
    printf("Negative: %d\n", word_psw.psw.n);
    printf("Overflow: %d\n", word_psw.psw.v);
    printf("Sleep: %d\n", word_psw.psw.slp);
    printf("Current Priority: %d\n", word_psw.psw.current);
    printf("Faulting: %d\n", word_psw.psw.faulting);
    printf("PRevious Priority: %d\n", word_psw.psw.previous);
}

void edit_psw_bits() {
    int choice;
    unsigned short new_bit;

    while (1) {
        printf("Choose a PSW bit to change: \n");
        printf("1. Carry\n2. Zero\n3. Negative\n4. Sleep Rate\n5. Overflow\n6. Current Priority\n7. Active Fault\n8. Reserved\n9. Previous Priority\n10. Quit\n");
        printf("ANS-> ");
        scanf_s("%d", &choice);

        switch (choice) {
        case 1: // Carry
            printf("Enter the new Carry bit (0 or 1): ");
            printf("ANS-> ");
            scanf_s("%hx", &new_bit);
            word_psw.psw.c = new_bit;
            break;
        case 2: // Zero
            printf("Enter the new Zero bit (0 or 1): ");
            printf("ANS-> ");
            scanf_s("%hx", &new_bit);
            word_psw.psw.z = new_bit;
            break;
        case 3: // Negative
            printf("Enter the new Negative bit (0 or 1): ");
            printf("ANS-> ");
            scanf_s("%hx", &new_bit);
            word_psw.psw.n = new_bit;
            break;
        case 4: // Sleep Rate
            printf("Enter the new Sleep Rate bit (0 or 1): ");
            printf("ANS-> ");
            scanf_s("%hx", &new_bit);
            word_psw.psw.slp = new_bit;
            break;
        case 5: // Overflow
            printf("Enter the new Overflow bit (0 or 1): ");
            printf("ANS-> ");
            scanf_s("%hx", &new_bit);
            word_psw.psw.v = new_bit;
            break;
        case 6: // Current Priority
            printf("Enter the new Current Priority (0 to 7): ");
            printf("ANS-> ");
            scanf_s("%hx", &new_bit);
            word_psw.psw.current = new_bit;
            break;
        case 7: // Active Fault
            printf("Enter the new Active Fault bit (0 or 1): ");
            printf("ANS-> ");
            scanf_s("%hx", &new_bit);
            word_psw.psw.faulting = new_bit;
            break;
        case 8: // Reserved
            printf("Enter the new Reserved bits (0 to 15): ");
            printf("ANS-> ");
            scanf_s("%hx", &new_bit);
            word_psw.psw.reserved = new_bit;
            break;
        case 9: // Previous Priority
            printf("Enter the new Previous Priority (0 to 7): ");
            printf("ANS-> ");
            scanf_s("%hx", &new_bit);
            word_psw.psw.previous = new_bit;
            break;
        case 10: // Quit
            break;
        default:
            printf("Invalid choice!\n");
            continue;
        }

        if (choice == 10) {
            break;
        }
    }

    printf("PSW updated successfully!\n");
}


void change_registers() {
    int reg = 0;
    unsigned short value = 0x00;
    char choice = 0;

    do {
        printf("Choose which register to change and new value (e.g., 1 2000):\n"
            "0->R0\n 1->R1\n 2->R2\n 3->R3\n 4->R4\n 5->R5\n 6->R6\n 7->R7\n Ans->");
        scanf_s("%d %hx", &reg, &value);

        if (reg > 7 && reg < 0) {
            printf("Choice not valid. Please choose again!");
            choice = 'y';
        }
        else {
            regfile[0][reg] = value; //register value changed
            printf("Change more register values? Y/N\n");
            scanf_s(" %c", &choice); // Space before %c to consume any whitespace characters
        }
    } while (choice == 'Y' || choice == 'y');
}

void cache_menu() {
    char choice;
    unsigned short address;
    int initialized = 0;

    while (1) {

        printf("Cache Menu!!\n");
        printf("e) Edit cache lines\n");
        printf("d) Display cache lines\n");
        printf("b) Go back to program menu\n");
        printf("ANS-> ");
        scanf_s(" %c", &choice);

        //int ch;
        //while ((ch = getchar()) != '\n' && ch != EOF);

        switch (choice) {
        case 'E':
        case 'e':
            edit_cache_lines();
            break;
        case 'D':
        case 'd':
            display_cache();
            break;
        case 'B':
        case 'b':
            return 0;
        default:
            printf("\nInvalid command, Try again!!!\n\n");
        }
    }
}

void debugger() {
    loader();

    while (1) {
        char choice1 = 0, ans = 0;
        cache_type = NO_CACHE;

        do {
            printf("\nProgram Menu!!\n");
            printf("t) CACHE TYPE\n");
            printf("g) Go to cache menu\n");
            printf("c) Run CPU\n");
            printf("b) Enter breakpoint\n");
            printf("m) Display data from memory\n");
            printf("r) Display CPU registers\n");
            printf("u) Display psw bits(updated or not)\n");
            printf("p) Change PC value\n");
            printf("v) Change register values\n");
            printf("w) Change psw bits\n");
            printf("s) Stop\n");
            printf("ANS-> ");
            scanf_s(" %c", &choice1);  // Added a space before %c to skip any leftover newline characters
            printf("\n");

            switch (choice1)
            {
            case 'T':
            case 't': //start emulator
                printf("Choose memory mode\n");
                printf("a) Cache - Associative organization\n");
                printf("d) Cache - Direct Mapping organization\n");
                printf("n) No Cache - Only memory\n");
                printf("Ans-> ");
                scanf_s(" %c", &ans);
                if (ans == 'A' || ans == 'a') {
                    init_cache();
                    printf("Cache memory initilaized!!\n");
                    cache_type = ASSOCIATIVE;
                }
                else if (ans == 'D' || ans == 'd') {
                    init_cache();
                    printf("Cache memory initilaized!!\n");
                    cache_type = DIRECTMAPPING;
                }
                else {
                    cache_type = NO_CACHE;
                }
                break;
            case 'C':
            case 'c': //start emulator
                initialize_signal_handler();
                run_xm();

                break;
            case 'B':
            case 'b': //start emulator
                break_point();
                break;
            case 'P':
            case 'p': //change pc value
                change_pc();
                break;
            case 'V':
            case 'v': //change register value. PC can also be changed here
                change_registers();
                break;
            case 'W':
            case 'w': //change register value. PC can also be changed here
                edit_psw_bits();
                break;
            case 'M':
            case 'm': //display data in memory
                display_memory();
                break;
            case 'R':
            case 'r': //display the values in the computer register
                display_CPU_registers();
                break;
            case 'U':
            case 'u': //display the values in the computer register
                display_psw_bits();
                break;
            case 'G':
            case 'g': //display the values in the computer register
                cache_menu();
                break;
            case 'S':
            case 's': //end simulation
                printf("goodbye");
                return;  //exit
            default:
                printf("\nwrong choice.Enter Again");
                break;
            }

        } while (choice1 != 's' && choice1 != 'S');  // Exit the loop when 's' or 'S' is entered
    }
}
