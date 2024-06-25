#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

size_t strlen(const char* str);
bool strcmp(const char* str1, const char* str2);
/* Copies contents of string 1 into string 2 with length len */
void strcpy(const char* str1, char* str2, size_t len);
char* to_upper(char* str);
char* to_lower(char* str);