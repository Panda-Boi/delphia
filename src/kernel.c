#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "terminal.h"

extern void main(){

    terminal_initialize();

    print("Hello World!");

    return;

}