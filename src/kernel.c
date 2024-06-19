#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "terminal.h"
#include "fat.h"

extern void main(){

    terminal_initialize();

    print("Hello World!\n");

    initialize_fat((int8_t*)0x500);

    return;

}