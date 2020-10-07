// Navid Bhuiyan 
// 3-08-2020

/*
This program emulates specific MIPS instructions
and tracks the values of the registers used.

Refer to Assignment 2 Spec for more information.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// DEFINED VALUES
// -----------------------------------------------------------------
#define hex_buffer_size 9

typedef struct instructions {
    // Assume a max of 1000 instructions
    uint32_t array[1000];
    int length;
    int latest_accessed;
} instructions_t;


// FUNCTIONS
// -----------------------------------------------------------------
void file_reader(FILE *file_pointer, struct instructions *ins_s);
int check_hex_invalid(char *filename, struct instructions *ins_s);
void print_program(struct instructions *ins_s);
void print_output(struct instructions *ins_s, uint32_t registers[32]);
void print_registers(struct instructions *ins_s, uint32_t registers[32]);

int main(int argc, char *argv[]) {
    
    // Open each file as argument
    for (int arg = 1; arg < argc; arg++) {
        
        char *filename = argv[arg];
        FILE *hex_file = fopen(filename, "r");
        
        // Stores the 32 bit instructions (ins)
        uint32_t registers[32] = {(uint32_t) 0};
        struct instructions *ins_s = malloc(sizeof(*ins_s));

        if (hex_file == NULL) {
            // Error
            printf("No such file or directory: '%s'\n", argv[arg]);
        } else {
            // Add function to pass through file pointer.
            file_reader(hex_file, ins_s);

            if (check_hex_invalid(filename, ins_s)) {
                break;
            }

            print_program(ins_s);
            print_output(ins_s, registers);
            print_registers(ins_s, registers);
            fclose(hex_file);
        }

        // Free struct
        free(ins_s);

    }

    return 0;

}


/* 
Extract instructions per line and store the hexadecimal address in instructions
array.
*/
void file_reader(FILE *file_pointer, struct instructions *ins_s) {

    // Create hex_buffer to store each line when collecting characters.
    int hex_buffer_i = 0;
    int instructions_i = 0;
    char hex_buffer[hex_buffer_size];
    uint32_t num_instruction = 0;

    int c = fgetc(file_pointer);
    while (c != EOF) {

        // Add number to hex_buffer according to instruction
        if (c == '\n' || hex_buffer_i == 8) {
            hex_buffer[hex_buffer_i] = '\0';
            hex_buffer_i = 0;
            num_instruction = (uint32_t) strtol(hex_buffer, NULL, 16);
            ins_s->array[instructions_i] = num_instruction;
            instructions_i++;
            // Move one character to next line (assuming file is properly 
            // formatted).
            c = fgetc(file_pointer);
            continue;
        } else if (c == 'c' && hex_buffer_i == 0) {
            num_instruction = (uint32_t) strtol("c", NULL, 16);
            ins_s->array[instructions_i] = num_instruction;
            instructions_i++;
            // Move two characters to next line (assuming file is properly 
            // formatted).
            c = fgetc(file_pointer);
            c = fgetc(file_pointer);
            continue;
        }

        // Add line to buffer.
        hex_buffer[hex_buffer_i] = c;
        hex_buffer_i++;
        c = fgetc(file_pointer);

    }

    // Add length.
    ins_s->length = instructions_i;

}

/*
Check if hex number is proper mips instruction.
It checks the start and last six bits of the hex code against an array
with valid starting and ending 6 bits for MIPS instructions.
*/
int check_hex_invalid(char *filename, struct instructions *ins_s) {

    // Setting up possible variables in MIPS instructions (ins).
    uint32_t temp_ins = 0;
    int start_ins = 0; // First 6 bits
    int end_ins = 0; // Last 6 bits
    int middle_zeroes = 0;
    // Valid starting and ending six bits for MIPS instructions.
    int ending_ins_codes[7] = {2, 12, 32, 34, 36, 37, 42};
    int starting_ins_codes[6] = {4, 5, 8, 10, 12, 13};

    int instructions_index = 0;
    int invalid_code = 0;
    while ((instructions_index < ins_s->length) && invalid_code == 0) {
        
        temp_ins = ins_s->array[instructions_index];
        start_ins = temp_ins >> 26;
        end_ins = temp_ins & 63;

        // Assume they are instructions using three registers 
        // (i.e. these instructions have their last 6 bits initialised).
        if (start_ins == 0 || start_ins == 28) {
            
            middle_zeroes = (temp_ins >> 6) << 27;

            // Middle zeroes are unique to this type of
            // MIPS instruction.
            if (middle_zeroes != 0) {
                invalid_code = 1;
            }

            // Check last 6 bits against array.
            invalid_code = 1;
            for (int i = 0; i < 7; i++) {
                if (end_ins == ending_ins_codes[i]) {
                    invalid_code = 0;
                    break;
                }
            }

        // Assume lui instruction.
        } else if (start_ins == 15) {
            
            if ((temp_ins << 6) >> 27 != 0) {
                invalid_code = 1;
            }

        // Assume instructions defined by two registers 
        // (i.e. instruction has initialised its first 6 bits)
        } else {

            // Check first 6 bits against array.
            invalid_code = 1;
            for (int i = 0; i < 7; i++) {
                if (start_ins == starting_ins_codes[i]) {
                    invalid_code = 0;
                    break;
                }
            }

        }

        instructions_index++;

    }

    // Remove offset.
    instructions_index--;

    if (invalid_code) {
        printf("%s:%d: invalid instruction code: %08x\n", filename, instructions_index, temp_ins);
    }

    return invalid_code;

}

/* 
Translates and prints program in 'plain' english.
*/
void print_program(struct instructions *ins_s) {

    // Setting up possible variables in MIPS instructions.
    uint32_t temp_ins = 0;
    // Start instuction number.
    uint32_t start_ins = 0;
    // End instruction number.
    uint32_t end_ins = 0;
    uint16_t I = 0;
    // Check complement I for negative integers.
    uint16_t cI = 0; 
    int s = 0;
    int t = 0;
    int d = 0;

    printf("Program\n");

    int instructions_index = 0;
    while (instructions_index < ins_s->length) {
        
        temp_ins = ins_s->array[instructions_index];
        start_ins = temp_ins >> 26;
        end_ins = temp_ins & 63;

        printf("%3d: ", instructions_index);

        // syscall.
        if (temp_ins == 12) {
            
            printf("syscall\n");

        // instructions with s, t and d.
        } else if (start_ins == 0 || start_ins == 28) {
            
            s = (temp_ins << 6) >> 27;
            t = (temp_ins << 11) >> 27;
            d = (temp_ins << 16) >> 27;

            if (end_ins == 2) {
                printf("mul  ");
            } else if (end_ins == 32) {
                printf("add  ");
            } else if (end_ins == 34) {
                printf("sub  ");
            } else if (end_ins == 36) {
                printf("and  ");
            } else if (end_ins == 37) {
                printf("or   ");
            } else {
                printf("slt  ");
            }
            
            printf("$%d, $%d, $%d\n", d, s, t);
        
        // I instructions.
        } else {
            
            s = (temp_ins << 6) >> 27;
            t = (temp_ins << 11) >> 27;
            I = (uint16_t) temp_ins;
            cI = ~I; 

            // Deal with negative int input.
            if (I > cI) {
                I = ~I + 1;
            }

            if (start_ins == 4) {
                printf("beq  ");
            } else if (start_ins == 5) {
                printf("bne  ");
            } else if (start_ins == 8) {
                printf("addi ");
            } else if (start_ins == 10) {
                printf("slti ");
            } else if (start_ins == 12) {
                printf("andi ");
            } else if (start_ins == 13) {
                printf("ori  ");
            } else {
                printf("lui  ");
            }

            // if lui.
            if ((temp_ins) >> 26 == 15) {
                printf("$%d, %hu\n", t, I);
            // if I is neg.
            } else if (I > cI){
                printf("$%d, $%d, -%hu\n", s, t, I);
            } else {
                printf("$%d, $%d, %hu\n", t, s, I);
            }

        }

        instructions_index++;

    }

}


/* 
Prints output of mips program.
*/
void print_output(struct instructions *ins_s, uint32_t registers[32]) {
    
    printf("Output\n");

    // Setting up possible variables in MIPS instructions.
    uint32_t temp_ins = 0;
    // Start instuction number.
    uint32_t start_ins = 0;
    // End instruction number.
    uint32_t end_ins = 0;
    uint16_t I = 0;
    // Check complement I for negative integers.
    uint16_t cI = 0; 
    int s = 0;
    int t = 0;
    int d = 0;
    int neg = 0;

    int instructions_index = 0;
    while (instructions_index < ins_s->length) {
        
        temp_ins = ins_s->array[instructions_index];
        start_ins = temp_ins >> 26;
        end_ins = temp_ins & 63;

        // syscall.
        if (temp_ins == 12) {
            
            // syscall options to access latest accessed 
            // register (excluding $2).
            if (registers[2] == 10) {
                break;
            } else if (registers[2] == 11) {
                printf("%c", registers[ins_s->latest_accessed]);
            } else if (registers[2] == 1) {
                printf("%d", registers[ins_s->latest_accessed]);
            // Detect error for unknown system calls.
            } else {
                printf("Unknown system call: %d\n", registers[2]);
                break;
            }

        // instructions with s, t and d.
        } else if (start_ins == 0 || start_ins == 28) {

            s = (temp_ins << 6) >> 27;
            t = (temp_ins << 11) >> 27;
            d = (temp_ins << 16) >> 27;

            // If register d is 0 or 2, don't change it.
            if (d == 0 || d == 2) {
                instructions_index++;
                continue;
            }

            // Instructions for mul, add, sub, and, or and slt.
            if (end_ins == 2) {
                registers[d] = registers[s] * registers[t];
            } else if (end_ins == 32) {
                registers[d] = registers[s] + registers[t];
            } else if (end_ins == 34) {
                registers[d] = registers[s] - registers[t];
            } else if (end_ins == 36) {
                registers[d] = registers[s] & registers[t];
            } else if (end_ins == 37) {
                registers[d] = registers[s] | registers[t];
            } else {
                if (registers[s] < registers[t]) {
                    registers[d] = 1;
                } else {
                    registers[d] = 0;
                }
            }

            // slt doesnt access memory.
            if (end_ins != 42) {
                ins_s->latest_accessed = d;    
            }

        // 'I' containing instructions.
        } else {
            
            s = (temp_ins << 6) >> 27;
            t = (temp_ins << 11) >> 27;
            I = (temp_ins << 11) >> 11;


            // Cannot change register 0.
            if (t == 0 && start_ins == 13) {
                instructions_index++;
                continue;
            }

            // Deal with negative int input.
            neg = 0;
            cI = ~I; 
            if (I > cI) {
                I = ~I + 1;
                neg = 1;
            }

            // beq instruction, where register index moves 
            // fowards or backwards
            if (start_ins == 4) {
                
                if (registers[s] == registers[t]) {
                    instructions_index = instructions_index - (int32_t)I;
                    continue;
                }
                
                
            // bne instruction, where register index moves 
            // fowards or backwards.
            } else if (start_ins == 5) {
                
                if (registers[s] != registers[t]) {
                    instructions_index = instructions_index - (int32_t) I;
                    continue;
                }
 
            // addi with special cases.
            } else if (start_ins == 8) {

                // If a negative number is supplied then substract it.
                if (neg == 1) {
                    registers[t] = registers[s] - (int32_t) I;
                } else {
                    registers[t] = registers[s] + I;
                }

            // slti, andi, ori, lui
            } else if (start_ins == 10) {
                registers[t] = registers[s] < I;
            } else if (start_ins == 12) {
                registers[t] = registers[s] & I;
            } else if (start_ins == 13) {
                registers[t] = registers[s] | I;
            } else {
                registers[t] = I << 16;
            }

            // Keep track of lastest acccess for syscall.
            // Avoid recording register 2.
            if (t != 2) {
                ins_s->latest_accessed = t;
            }
        }

        instructions_index++;
    }

}

/*
Prints registers with non-zero values.
*/
void print_registers(struct instructions *ins_s, uint32_t registers[32]) {
    
    printf("Registers After Execution\n");
    for (int register_i = 0; register_i < 32; register_i++) {
        if (registers[register_i] > 0) {
            printf("$%-2d %s %d\n", register_i, "=", registers[register_i]);
        }

    }
}
