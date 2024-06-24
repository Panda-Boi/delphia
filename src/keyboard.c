#include "keyboard.h"

char scancode_set[] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '-', '=', 8, 9, 'q', 'w', 'e', 'r', 't', 'y', 'u','i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0, 0, ' ', 0 
};

#define CONTROLLER_PORT 0x64

bool keyboard_input_ready();

char keyboard_getinput() {
    while (!keyboard_input_ready());

    size_t scancode = x86_inb(0x60);

    if (scancode < 0x3b) {
        return scancode_set[scancode];
    }

    return 0;

}

bool keyboard_input_ready() {
    return x86_inb(CONTROLLER_PORT) & 1;
}