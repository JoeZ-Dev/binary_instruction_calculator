#include <stdio.h>
#include <string.h>

#define NUM_REGS 22
#define HIST_REGS 10
#define NAME_MAX 64
#define DISPLAY_MAX 128

/* 
Simple project designed around a binary calculator that uses an instruction set
similar to MIPS assembly language.  The calculator will have registers to store binary
numbers and history of calculations. The instruction set will include operations like 
addition, subtraction, multiplication, and division.

The structure was inspired by a previously completed Codecademy Python project but 
implemented in C for learning purposes.  It is over-commented for educational clarity.

There are 4 layers of abstraction here:
1) The input instruction binary set (binary)
2) Placement of bits for opcode, source registers, and store register (binary)
3) Decoding of instructions and opcodes (from binary to integer)
4) Calculator values and operations (interpreting integers to perform operations)

*/

/* ~~~~ CODECADEMY PROJECT DESCRIPTION ~~~~
#  System Design:
#   - Four function calculator
#   - Can only operate on numbers stored in registers
#   - Processor receives binary data as 32-bit strings
#   - Returns results to the terminal
#   - Can operate on 10-bit numbers (0 thru 1023)
#   - Results can be negative (5 - 10 = -5)
#  Instruction format:
#   - 32 bit's in length
#   - Binary data will come to the CPU as a string
#   - Registers (32 total on CPU, 0-indexed)
#      - 0 thru 21:  Available for number storage
#        - 0: Constant 0
#      - 22 thru 31: Available for history storage
# +=======+=======+=======+=======+=======+=======+=======+=======+
# | 0: 0  | 1:    | 2:    | 3:    | 4:    | 5:    | 6:    | 7:    |
# +-------+-------+-------+-------+-------+-------+-------+-------+
# | 8:    | 9:    |10:    |11:    |12:    |13:    |14:    |15:    |
# +-------+-------+-------+-------+-------+-------+-------+-------+
# |16:    |17:    |18:    |19:    |20:    |21:    |22: H0 |23: H1 |
# +-------+-------+-------+-------+-------+-------+-------+-------+
# |24: H2 |25: H3 |26: H4 |27: H5 |28: H6 |29: H7 |30: H8 |31: H9 |
# +=======+=======+=======+=======+=======+=======+=======+=======+
#   - Bits 0-5 are OPCODEs
#     - use variable 'opcode' in program
#   - Bits 6-10 & 11-15 are source register locations
#     - use variables 'source_one' and 'source_two' in program
#   - Bits 16-25 are reserved for adding a new value to the registers
#     - use variable 'store' in program
#   - Bits 26-31 are functions
#     - use variable 'function_code' in program
# +--------+----------+-------------------------------------+
# | OPCODE | FUNCTION | Definition                          |
# | 000000 |  100000  | Add two numbers from registers      |
# | 000000 |  100010  | Subtract two numbers from registers |
# | 000000 |  011000  | Multiply two numbers from registers |
# | 000000 |  011010  | Divide two numbers from registers   |
# | 000001 |  000000  | Store value to next register        |
# | 100001 |  000000  | Return previous calculation         |
# +--------+----------+-------------------------------------+

*/




// Define our Binary Calculator structure
typedef struct {  
    char name[NAME_MAX];

    // number_registers in Python: stores binary strings like "1010"
    // We'll store them as strings too (simpler while learning).
    // 10-bit store field => up to 10 chars, plus '\0' => 11.
    int number_registers[NUM_REGS];

    // history_registers in Python stores bin(result) strings.
    // We'll store as strings too for now.
    int history_registers[HIST_REGS]; // enough for "-0b..." etc.

    int numbers_index;        // starts at 1 in your Python
    int history_index;        // starts at 0
    int temp_history_index;   // used for "get last calculation"

    char user_display[DISPLAY_MAX];
} BinCal;

// Define our Decoded Instruction structure
typedef struct{  // Decoded instruction structure
    int opcode;   // - Bits 0-5 are OPCODEs
    int source_one; // - Bits 6-10 are SOURCE ONE
    int source_two; // - Bits 11-15 are SOURCE TWO
    int store;  // - Bits 16-25 are reserved for adding a new value to the registers
    int function_code; // - Bits 26-31 are FUNCTION CODES

} Decoded;



// Function Prototypes
static void bc_init(BinCal *calc, const char *name);
static void update_display(BinCal *calc, const char *msg);
static int read_bits(const char *instr, int start, int length);
static int decode_and_validate(const char *instruction, Decoded *d);
static void execute_opcode(BinCal *calc, Decoded d);
static void execute_alu_instructions(BinCal *calc, Decoded d);
static void error_processing(const int error_code, BinCal *calc);   
static void store_to_register(BinCal *calc, Decoded d); 
static int load_value_from_register(BinCal *calc, int register_index); //
static void store_to_history(BinCal *calc, int value);
static int retrieve_last_calculation(BinCal *calc);
static void add(BinCal *calc, Decoded d);
static void subtract(BinCal *calc, Decoded d);
static void multiply(BinCal *calc, Decoded d);
static void divide(BinCal *calc, Decoded d);

// Function to retrieve the last calculation from history registers
static int retrieve_last_calculation(BinCal *calc){
    if (calc->temp_history_index == 0){
        calc->temp_history_index = calc->history_index - 1; //set to last stored calculation
    }
    else {
        calc->temp_history_index -= 1; //decrement to get previous calculation
        if (calc->temp_history_index < 0){ //wrap around if we go below 0
            calc->temp_history_index = HIST_REGS - 1;
        }
    }
    int value = calc->history_registers[calc->temp_history_index];
    char msg[DISPLAY_MAX];
    snprintf(msg, sizeof(msg), "Retrieved previous calculation: %d\n", value);
    update_display(calc, msg);
    return value;
}

// Function to store a value into history registers
static void store_to_history(BinCal *calc, int value){
    if (calc->history_index > (HIST_REGS - 1)){ //check if we exceed history limit
        calc->history_index = 0; //wrap around if we exceed history limit
    }
    calc->history_registers[calc->history_index] = value; //store value into next history register
    calc->history_index += 1; //increment index for next store
}

// Fuction to store a value into a register, opcode 000001
static void store_to_register(BinCal *calc, Decoded d){
    if (calc->numbers_index > (NUM_REGS - 1)){ //check if we exceed register limit
        calc->numbers_index = 1; //wrap around if we exceed register limit
    }
    calc->number_registers[0] = 0; //register 0 is always 0

    calc->number_registers[calc->numbers_index] = d.store; //store value into next register
    calc->numbers_index += 1; //increment index for next store
    update_display(calc, "Value stored to register successfully.\n");

}

// Function to load a value from number register
static int load_value_from_register(BinCal *calc, int register_index){
    int i = calc->number_registers[register_index];
    return i;
}

// Functions for calculations
static void add(BinCal *calc, Decoded d){
    int val1 = load_value_from_register(calc, d.source_one);  //load value 1
    int val2 = load_value_from_register(calc, d.source_two);  //load value 2
    int result = val1 + val2;  //perform addition
    store_to_history(calc, result);  //store result to history
    char msg[DISPLAY_MAX];  //prepare display message
    snprintf(msg, sizeof(msg), "Addition result: %d\n", result);  //format message to avoid length issues
    update_display(calc, msg); //update display with result
}

static void subtract(BinCal *calc, Decoded d){
    int val1 = load_value_from_register(calc, d.source_one);
    int val2 = load_value_from_register(calc, d.source_two);
    int result = val1 - val2;
    store_to_history(calc, result);
    char msg[DISPLAY_MAX];
    snprintf(msg, sizeof(msg), "Subtraction result: %d\n", result);
    update_display(calc, msg);
}

static void multiply(BinCal *calc, Decoded d){
    int val1 = load_value_from_register(calc, d.source_one);
    int val2 = load_value_from_register(calc, d.source_two);
    int result = val1 * val2;
    store_to_history(calc, result);
    char msg[DISPLAY_MAX];
    snprintf(msg, sizeof(msg), "Multiplication result: %d\n", result);
    update_display(calc, msg);
}

static void divide(BinCal *calc, Decoded d){
    int val1 = load_value_from_register(calc, d.source_one);
    int val2 = load_value_from_register(calc, d.source_two);
    if (val2 == 0){     //Do not allow division by zero
        update_display(calc, "Error: Division by zero\n");
        return;
    }
    int result = val1 / val2;
    store_to_history(calc, result);
    char msg[DISPLAY_MAX];
    snprintf(msg, sizeof(msg), "Division result: %d\n", result);
    update_display(calc, msg);
}


/*  Separates and converts the binary instructions into our Decoded structure,
    returns 0 if successful, > 0 for indicating error code.
    1 = invalid instruction length
    2 = invalid instrunction characters
    3 = invalid opcode/function code combination
    4 = invalid function code
    5 = invalid opcode
    6 = invalid register index
*/ 
static int decode_and_validate(const char *instruction, Decoded *d){

    if (strlen(instruction) != 32){
        return 1; //invalid instruction length
    }

    for (int i = 0; i < 32; i++){
        if (instruction[i] != '0' && instruction[i] != '1'){
            return 2; //invalid instruction characters
        }
    }

    d->opcode = read_bits(instruction, 0, 6);
    d->source_one = read_bits(instruction, 6, 5);
    d->source_two = read_bits(instruction, 11, 5);
    d->store = read_bits(instruction, 16, 10);
    d->function_code = read_bits(instruction, 26, 6);
    //opcode and function validation
    if (d->opcode == 0){  
        switch (d->function_code){  //check valid function codes for opcode 0
            case 32:  // 0b100000: Add
            case 34:  // 0b100010: Subtract
            case 24:  // 0b011000: Multiply
            case 26:  // 0b011010: Divide
                break;
            default:
                return 4; //invalid function code
        }
        if (d->source_one < 1 || d->source_one > 21 || d->source_two < 1 || d->source_two > 21){
        return 6; //invalid register index
    }

    }
    else{
        switch (d->opcode){
            case 1: //  0b000001 Store number
                if (d->function_code != 0){
                    return 3; //invalid opcode/function code combination
                }
                if (d->source_one != 0 || d->source_two != 0){
                    return 6; //invalid register index
                }
                break;
            case 33: //0b100001 Return previous calculation
                if (d->function_code != 0){
                    return 3; //invalid opcode/function code combination
                }
                if (d->source_one != 0 || d->source_two != 0){
                    return 6; //invalid register index
                }
                break;
            default:
                return 5; //invalid opcode
        }
        
    }

    //register index validation
    //if source one or source two are > 0, check to see if they are valid register indices
    // also check to make sure it's opcode 0 or else source registers are invalid



    return 0; //success
}

// Decodes instructions based on function code, executes operations
static void execute_alu_instructions(BinCal *calc, Decoded d){  
    switch (d.function_code){ 
        case 32:  // 0b100000: Add
            add(calc, d);
            break;
        case 34:  // 0b100010: Subtract
            subtract(calc, d);
            break;
        case 24:  // 0b011000: Multiply
            multiply(calc, d);
            break;
        case 26:  // 0b011010: Divide
            divide(calc, d);
            break;
        default:
            printf("Invalid instruction");
            break;
        }
}

// Decodes opcode and calls appropriate function
static void execute_opcode(BinCal *calc, Decoded d){
    switch (d.opcode){ 
        case 0: //  0b000000 Call decode_instructions
            execute_alu_instructions(calc, d);
            break;
        case 1: //  0b000001 Store number
            store_to_register(calc, d); 
            break;
        case 33: //0b100001 Return previous calculation
            retrieve_last_calculation(calc);
            break;
        default:
            printf("Invalid opcode");
            break;
        }
}

// Update the user display with a message
static void update_display(BinCal *calc, const char *msg) {
    // Save + print 
    strncpy(calc->user_display, msg, DISPLAY_MAX - 1);
    calc->user_display[DISPLAY_MAX - 1] = '\0';
    puts(calc->user_display);
}

// Initialize the Binary Calculator
static void bc_init(BinCal *calc, const char *name) {
    // Clear the struct
    memset(calc, 0, sizeof(*calc));

    // Set defaults _
    strncpy(calc->name, name, NAME_MAX - 1);
    calc->name[NAME_MAX - 1] = '\0';

    calc->numbers_index = 1;
    calc->history_index = 0;
    calc->temp_history_index = 0;

    // Register 0 is always constant 0
    calc->number_registers[0] = 0;

    char welcome[DISPLAY_MAX];
    snprintf(welcome, sizeof(welcome), "Welcome, %s", calc->name);
    update_display(calc, welcome);
}

// Error processing function, prints error message based on code
static void error_processing(const int error_code, BinCal *calc){
    switch (error_code){
        case 1:
            update_display(calc, "Error: Invalid instruction length\n");
            break;
        case 2:
            update_display(calc,"Error: Invalid instruction characters\n");
            break;
        case 3:
            update_display(calc, "Error: Invalid opcode/function code combination\n");
            break;
        case 4:
            update_display(calc, "Error: Invalid function code\n");
            break;
        case 5:
            update_display(calc, "Error: Invalid opcode\n");
            break;
        case 6:
            update_display(calc, "Error: Invalid register index\n");
            break;
        default:
            update_display(calc, "Error: Unknown error code\n");
            break;
    }
}

// Reads bits from instruction string at specific location and returns them converted to integer
static int read_bits(const char *instr, int start, int length) {
    int value = 0;  //return value

    for (int i = 0; i < length; i++) {  //iterate through each bit from our instruction until we reach length

        value = (value * 2) + (instr[start + i] - '0'); // shift left and add bit. -0 to convert char to int
    }

    return value;
}

int main(void) {

    //Begin Welcome screen.
    printf("Binary Calculator Starting, please state your name to load:\n");
    char name[NAME_MAX];
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';  //remove newline character from fgets

    //Initialize Calculator
    BinCal calc;
    bc_init(&calc, name);
    Decoded d; 
    char instruction[32 + 2]; //32 bits + newline + null terminator
    while (1)  {  //Start our input loop
        printf("Please enter a 32-bit binary instruction:\n");
        

        if (fgets(instruction, sizeof(instruction), stdin) == NULL) {
             break; // EOF or input error
        }
        instruction[strcspn(instruction, "\n\r")] = '\0';  //remove newline character from fgets
        if (instruction[0] == 'q'){
            break; //exit loop if user enters 'q'
        }

        int status = decode_and_validate(instruction, &d);
        if (status != 0){
            error_processing(status, &calc);
            continue; //skip to next iteration if error
        }
        else {
            update_display(&calc, "Instruction valid, executing...\n");
            execute_opcode(&calc, d);
        }  
    }

    return 0;
}

/*

 Adds 10 and 5 to number registers
00000100000000000000001010000000
00000100000000000000000101000000

 Adds/Subtracts/Multiplies/Divides 5 and 10 from registers
00000000001000100000000000100000
00000000001000100000000000100010
00000000001000100000000000011000
00000000001000100000000000011010

# Gets the last calculations
10000100000000000000000000000000

   //For debugging, we are goign to hardcode an instruction string
    // then print it, read_bits, and print the results.
    
    
    //const char *instruction = "10000100000000000000000000000000";
    const char *instruction = "00000100000000000000000101000000"; //store 5 into register

    int opcode = read_bits(instruction, 0, 6);
    int src1   = read_bits(instruction, 6, 5);
    int src2   = read_bits(instruction, 11, 5);
    int store  = read_bits(instruction, 16, 10);
    int function   = read_bits(instruction, 26, 6);
    printf("first6=%.6s\n", instruction);
    printf("opcode = %d\n", opcode);
    printf("src1   = %d\n", src1);
    printf("src2   = %d\n", src2);
    printf("store   = %d\n", store);
    printf("function   = %d\n", function);
    */

