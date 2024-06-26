#include "shell.h"

typedef enum {
    ERROR = 0,
    EXIT = 1,
    ECHO = 2,
    HELP = 3,
    CLEAR = 4,
    DIR = 5,
    CAT = 6,
} com_type;

typedef struct {
    com_type type;
    size_t argc;
    char* argv;
} command;

command parse_command();
void run_command(command com);

void exit();
void echo(command com);
void help();
void clear();
void dir();
void cat(command cmd);

char* current_command;
char* command_head;

bool running = true;

void initialize_shell(void* buffer, DISK disk) {

    char prompt[4] = {0, ':', '>', '\0'};

    current_command = (char*) buffer;
    command_head = current_command;

    switch (disk.id) {
    case 0:
        prompt[0] = 'A';
        break;
    case 1:
        prompt[0] = 'B';
    case 80:
        prompt[0] = 'C';
    case 81:
        prompt[0] = 'D';
    }

    print("\n\n");
    print(prompt);

    while (running) {
        char c = keyboard_getinput();
        if (c) {

            if (c == '\b') {
                if (command_head != current_command) {
                    command_head--;
                    printc(c);
                }
                continue;
            }

            // echo char
            printc(c);

            // add to current command
            *command_head = c;
            command_head++; 

            if (c == '\n') {

                // parse the current command
                command com = parse_command();
                command_head = current_command;

                run_command(com);

                if (!running) {
                    break;
                }
                print(prompt);
            } 
        }
    }

    return;
}

command parse_command() {

    // replace the final \n with a \0 and make it uppercase
    command_head--;
    *command_head = '\0';

    command com;
    com.argc = strtok(current_command, ' ');
    com.argv = current_command;

    current_command = to_upper(current_command);

    if (strcmp(current_command, "EXIT")) {
        com.type = EXIT;
    } else if (strcmp(current_command, "ECHO")) {
        com.type = ECHO;
    } else if (strcmp(current_command, "HELP")) {
        com.type = HELP;
    } else if (strcmp(current_command, "CLEAR")) {
        com.type = CLEAR;
    } else if (strcmp(current_command, "DIR")) {
        com.type = DIR;
    } else if (strcmp(current_command, "CAT")) {
        com.type = CAT;
    } else {
        com.type = ERROR;
    }

    return com;

}

void run_command(command com) {

    switch (com.type) {
    case ERROR:
        print("Command not found\n");
        break;
    case EXIT:
        exit();
        break;
    case ECHO:
        echo(com);
        break;
    case HELP:
        help();
        break;
    case CLEAR:
        clear();
        break;
    case DIR:
        dir();
        break;
    case CAT:
        cat(com);
        break;
    }

}

void exit() {
    print("Exiting the shell...\n");
    running = false;
}

void echo(command com) {

    char* args = com.argv;
    args += strlen(args) + 1;

    for (int i=0;i<com.argc;i++) {

        print(args);

        printc(' ');

        args += strlen(args) + 1;

    }

    printc('\n');

}

void help() {

    print("If you need help you're already fucked sorry\n");
    exit();

}

void clear() {

    terminal_clear();

}

void dir() {

    size_t i = 1;
    while (true) {
        
        ROOT_DIR_ENTRY* entry = file_id(i);

        if (!entry->first_cluster_low) {
            break;
        }

        char name[12];
        strcpy(entry->name, name, 11);
        name[11] = '\0';

        print(name);
        print("  ");
        
        print_int(entry->size);
        print(" bytes");
        print("\n");

        i++;

    }

    print_int(i-1);
    print(" FILE(S)\n");

}

void cat(command cmd) {

    if (cmd.argc != 2) {
        print("Incorrect Usage...\n");
        return;
    }

    // get second argument
    char* file_name = cmd.argv;
    file_name += strlen(cmd.argv) + 1;
    file_name = to_upper(file_name);

    char* buffer = command_head;

    ROOT_DIR_ENTRY* file = file_find(file_name);

    if (!file_read(file_name, buffer) || !file) {
        print("File named ");
        print(file_name);
        print(" not found...\n");
        return;
    }

    // byte dumping instead of printing
    size_t len = file->size;
    for (int i=0;i<len;i++) {
        putc(buffer[i]);
    }

}