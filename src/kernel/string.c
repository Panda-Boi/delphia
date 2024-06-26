#include "string.h"

// returns length of a string
size_t strlen(const char* str) {

    size_t len = 0;

    while (str[len]) {
        len++;
    }

    return len;
}

bool strcmp(const char* str1, const char* str2) {

    while (*str1 && *str2) {
        if (*str1 != *str2) {
            return false;
        }
        str1++;
        str2++;
    }

    if (*str1 || *str2) {
        return false;
    }

    return true;

}

void strcpy(const char* str1, char* str2, size_t len) {

    for (int i=0;i<len;i++) {
        *str2 = *str1;
        str1++;
        str2++;
    }
    
}

size_t strtok(char* str, char delimiter) {

    size_t count = 0;

    size_t len = strlen(str);
    size_t i = 0;
    char* current_token = str;

    while (*str) {
        if (*str == delimiter) {

            // crushing multiple delimeters into 1 char
            if (i == 0 || *(str-1) == '\0') {
                strcpy(str+1, current_token, len - i);
                continue;
            }

            *str = '\0';
            str++;
            i++;
            current_token = str;

            count++;
            continue;

        }

        str++;
        i++;
    }

    count++;

    return count;

}

char* to_upper(char* str) {

    char* upper_str = str;
    while (*upper_str) {
        if (*upper_str >= 97 && *upper_str <= 122) {
            *upper_str -= 32;
        }
        upper_str++;
    }

    return str;

}

char* to_lower(char* str) {

    char* lower_str = str;
    while (*lower_str) {
        if (*lower_str >= 65 && *lower_str <= 90) {
            *lower_str += 32;
        }
        lower_str++;
    }

    return str;

}