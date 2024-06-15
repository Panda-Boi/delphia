#include <stdint.h>
#include <stddef.h>

// VGA Hardware text mode color constants
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

void terminal_initialize();
void terminal_setcolor(uint8_t color);
void terminal_newline();
void terminal_clear();
void terminal_scroll();

void print(char* str);
void putc(char c);
void puts(char* str, size_t len);

void put_vga_entry_at(uint16_t entry, size_t x, size_t y);
uint16_t get_vga_entry_at(size_t x, size_t y);

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void disable_cursor(); // asm func
void update_cursor(int x, int y);

void screen_clear();

size_t strlen(const char* str);

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg);
static inline uint16_t vga_entry(unsigned char uc, uint8_t color);

static inline void outb(uint16_t port, uint8_t val);
static inline uint8_t inb(uint16_t port);

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
	outb(0x3D4, 0x0A);
	outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);
 
	outb(0x3D4, 0x0B);
	outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

void disable_cursor() {
	outb(0x3D4, 0x0A);
	outb(0x3D5, 0x20);
}


void update_cursor(int x, int y) {
	uint16_t pos = y * VGA_WIDTH + x;
 
	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

// returns length of a string
size_t strlen(const char* str) {

    size_t len = 0;

    while (str[len]) {
        len++;
    }

    return len;
}

// returns vga color byte from foreground and background colors
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
	return fg | bg << 4;
}

// returns 2 byte text mode entry from char and color byte
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
    /* There's an outb %al, $imm8 encoding, for compile-time constant port numbers that fit in 8b. (N constraint).
     * Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
     * The  outb  %al, %dx  encoding is the only option for all other cases.
     * %1 expands to %dx because  port  is a uint16_t.  %w1 could be used if we had the port number a wider C type */
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ( "inb %w1, %b0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}