#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

size_t strlen(const char* str);
bool strcmp(const char* str1, const char* str2);

/* Copies contents of string 1 into string 2 with length len */
void strcpy(const char* str1, char* str2, size_t len);

/* Tokenizes based on the delimiter and returns number of total words 
Set nullify to true to replace all delimiters with \0 */
size_t strtok(char* str, char delimiter, bool nullify);
/* Returns a pointer to the first occurence of a character
Returns Null if the character does not occur in the string*/
char* strchr(char* str, char c);

char* to_upper(char* str);
char* to_lower(char* str);

bool is_int(char c);