#include "../include/assembler.h"

int main(int argc, char *argv[]) {
    return Assembler::singleton().pass(argc, argv);
}

// isr_timer 
// isr_terminal 
// my_counter 
// my_start 
// handler 