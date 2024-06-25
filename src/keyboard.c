#include "keyboard.h"

#define ESC 0x01
#define LCTRL 0x1D
#define LALT 0x38
#define LSHFT 0x2A
#define RSHFT 0x36
#define CAPSLOCK 0x3A
#define LCTRLOUT 0x9D
#define LALTOUT 0xB8
#define LSHIFTOUT 0xAA
#define RSHFTOUT 0xB6

#define NULLCHAR 0

char scancode_set[] = {
    NULLCHAR, NULLCHAR, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '-', '=', '\b', '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u','i', 'o', 'p', '[', ']', '\n', NULLCHAR,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', NULLCHAR, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', NULLCHAR, '*', NULLCHAR, ' ', NULLCHAR
};

char shifted_scancode_set[] = {
    NULLCHAR, NULLCHAR, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
    '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U','I', 'O', 'P', '{', '}', '\n', NULLCHAR,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', NULLCHAR, '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', NULLCHAR, '*', NULLCHAR, ' ', NULLCHAR,  
};

char capslock_scancode_set[] = {
    NULLCHAR, NULLCHAR, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '-', '=', '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U','I', 'O', 'P', '[', ']', '\n', NULLCHAR,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', NULLCHAR, '\\',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', NULLCHAR, '*', NULLCHAR, ' ', NULLCHAR,  
};

#define CONTROLLER_PORT 0x64

bool ctrl = false;
bool alt = false;
bool shift = false;
bool capslock = false;

bool keyboard_input_ready();

char keyboard_getinput() {
    while (!keyboard_input_ready());

    size_t scancode = x86_inb(0x60);

    switch (scancode) {
    case ESC:
        break;
    case LCTRL:
        ctrl = true;
        break;
    case LCTRLOUT:
        ctrl = false;
        break;
    case LALT:
        alt = true;
        break;
    case LALTOUT:
        alt = false;
        break;
    case LSHFT:
        shift = true;
        break;
    case RSHFT:
        shift = true;
        break;
    case LSHIFTOUT:
        shift = false;
        break;
    case RSHFTOUT:
        shift = false;
        break;
    case CAPSLOCK:
        capslock = !capslock;
        break;
    }

    // todo handle function keys later
    if (scancode > 0x3A) {
        return 0;
    }

    if (shift) {
        return shifted_scancode_set[scancode];
    } else if (capslock) {
        return capslock_scancode_set[scancode];
    } else {
        return scancode_set[scancode];
    }

}

// checks bit 0 of the 8042 ps/2 controller
bool keyboard_input_ready() {
    return x86_inb(CONTROLLER_PORT) & 1;
}