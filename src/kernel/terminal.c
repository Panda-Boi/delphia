#include "terminal.h"

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

// basic print function
void print(char* str) {

    puts(str, strlen(str));

}

// prints a character
void printc(char c) {

    putc(c);
    update_cursor(terminal_column, terminal_row);

}

size_t numOfDigits(size_t integer, uint8_t base) {

    if (integer < base) {
        return 1;
    }

    return numOfDigits(integer / base, base) + 1;

}

void print_int(size_t integer) {

    size_t len = numOfDigits(integer, 10);
    uint8_t digits[len];

    for (int i=0;i<len;i++) {
        size_t temp = integer;
        for (int j=len-i-1;j>0;j--) {
            temp /= 10;
        }
        digits[i] = temp % 10;
    }

    for (int i=0;i<len;i++) {
        putc(digits[i] + 48);
    }

    update_cursor(terminal_column, terminal_row);

}

void print_hex(size_t integer) {

    size_t len = numOfDigits(integer, 16);
    uint8_t digits[len];

    for (int i=0;i<len;i++) {
        size_t temp = integer;
        for (int j=len-i-1;j>0;j--) {
            temp /= 16;
        }
        digits[i] = temp % 16;
    }

    for (int i=0;i<len;i++) {
        if (digits[i] <= 9) {
            putc(digits[i] + 48);
        } else {
            putc(digits[i] + 55);
        }
    }

    update_cursor(terminal_column, terminal_row);

}

// initializes the terminal and clears the screen
void terminal_initialize() {

    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t*) 0xb8000; // vga text mode video memory address

    screen_clear();

}

// sets the terminal color
void terminal_setcolor(uint8_t color) {
	terminal_color = color;
}

// moves the current character cell to the next line
void terminal_newline() {

    terminal_column = 0;
    if (++terminal_row == VGA_HEIGHT) {
        terminal_scroll();
    }

}

void terminal_backspace() {

    // do nothing if at the start of the line
    if (terminal_column == 0 ) {
        return;
    }

    terminal_column--;
    const uint16_t entry = vga_entry(' ', terminal_color);
    put_vga_entry_at(entry, terminal_column, terminal_row);

}

// scrolls the terminal by shifting all rows up by 1
void terminal_scroll() {

    // 2 bytes * 25 characters
    for (size_t i = 0;i < VGA_HEIGHT - 1;i++) {
        for (size_t j = 0;j < VGA_WIDTH;j++) {
            // get entry at the next row
            const uint16_t next_entry = get_vga_entry_at(j, i+1);

            // set entry at current row equal to entry at the next row
            put_vga_entry_at(next_entry, j, i);
        }
    }

    //clear last row
    for (size_t i = 0;i < VGA_WIDTH;i++) {
        const uint16_t entry = vga_entry(' ', vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
        put_vga_entry_at(entry, i, VGA_HEIGHT - 1);
    }

    terminal_row = VGA_HEIGHT - 1;

}

// resets the current character cell and clears the terminal 
void terminal_clear() {

    terminal_row = 0;
    terminal_column = 0;

    screen_clear();

}

// clears the screen
void screen_clear() {

    for (size_t i = 0;i < VGA_HEIGHT;i++){
        for (size_t j = 0;j < VGA_WIDTH;j++) {

            // (i, j) -> coordinates of any given character cell
            terminal_buffer[i * VGA_WIDTH + j] = vga_entry(' ', terminal_color);
        
        }
    }

}

// writes a string starting at the current character cell
void puts(char* str, size_t len) {

    for (int i = 0;i < len;i++) {
        putc(str[i]);
    }

    update_cursor(terminal_column, terminal_row);
}

// writes a character at the current character cell
void putc(char c) {

    // check for special characters
    switch (c) {
        case '\n':
            terminal_newline();
            return;
        case '\b':
            terminal_backspace();
            return;
    }

    // write the character at the current terminal column and row indices
    const uint16_t entry = vga_entry(c, terminal_color);
    put_vga_entry_at(entry, terminal_column, terminal_row);

    // update terminal column and row indices
    if (++terminal_column == VGA_WIDTH) {

        terminal_newline();

    }

}

// writes a vga_entry to the vga video memory buffer at given coordinates (x, y)
void put_vga_entry_at(uint16_t entry, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = entry;
}

uint16_t get_vga_entry_at(size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    const uint16_t entry = terminal_buffer[index];
    return entry;
}

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
	x86_outb(0x3D4, 0x0A);
	x86_outb(0x3D5, (x86_inb(0x3D5) & 0xC0) | cursor_start);
 
	x86_outb(0x3D4, 0x0B);
	x86_outb(0x3D5, (x86_inb(0x3D5) & 0xE0) | cursor_end);
}

void disable_cursor() {
	x86_outb(0x3D4, 0x0A);
	x86_outb(0x3D5, 0x20);
}


void update_cursor(int x, int y) {
	uint16_t pos = y * VGA_WIDTH + x;
 
	x86_outb(0x3D4, 0x0F);
	x86_outb(0x3D5, (uint8_t) (pos & 0xFF));
	x86_outb(0x3D4, 0x0E);
	x86_outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

// returns vga color byte from foreground and background colors
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
	return fg | bg << 4;
}
    
// returns 2 byte text mode entry from char and color byte
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}