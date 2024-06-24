#include <stdint.h>
#include <stddef.h>

#include "string.h"

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
void print_int(size_t integer);
void print_hex(size_t integer);
void putc(char c);
void puts(char* str, size_t len);

void put_vga_entry_at(uint16_t entry, size_t x, size_t y);
uint16_t get_vga_entry_at(size_t x, size_t y);

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void disable_cursor(); // asm func
void update_cursor(int x, int y);

void screen_clear();

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg);
static inline uint16_t vga_entry(unsigned char uc, uint8_t color);

static inline void outb(uint16_t port, uint8_t val);
static inline uint8_t inb(uint16_t port);