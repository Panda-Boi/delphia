#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "terminal.h"

extern void main(uint32_t number){

    terminal_initialize();

    if (number == 69) {
        print("The number is indeed 69\n");
    }

    print("Hello World!");

    return;

}