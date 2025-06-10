#include "shell.h"

typedef enum {
    ERROR = 0,
    EXIT = 1,
    ECHO = 2,
    HELP = 3,
    CLEAR = 4,
    DIR = 5,
    CAT = 6,
    TOUCH = 7,
    RM = 8,
    EDIT = 9
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
void touch(command cmd);
void remove(command cmd);
void cat(command cmd);
void edit(command cmd);

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
                    *command_head = 0;
                    command_head--;
                    
                    // echo char
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
                size_t len = command_head - current_command;
                for (int i=0;i<len;i++) {
                    command_head[i] = 0;
                }
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
    com.argc = strtok(current_command, ' ', true);
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
    } else if (strcmp(current_command, "TOUCH")) {
        com.type = TOUCH;
    } else if (strcmp(current_command, "RM")) {
        com.type = RM;
    } else if (strcmp(current_command, "EDIT")) {
        com.type = EDIT;
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
    case TOUCH:
        touch(com);
        break;
    case RM:
        remove(com);
        break;
    case EDIT:
        edit(com);
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
    size_t count = 0;
    while (true) {
        
        DIR_ENTRY* entry = file_id(i);

        if (!entry->first_cluster_low) {

            // deleted entry
            if (entry->name[0] == 0xE5) {
                i++;
                continue;
            }

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

        count++;
        i++;

    }

    print_int(count);
    print(" FILE(S)\n");

}

void touch(command cmd) {
    if (cmd.argc != 2) {
        print("Incorrect Usage...\n");
        return;
    }

    // get second argument
    char* file_name = cmd.argv;
    file_name += strlen(cmd.argv) + 1;
    file_name = to_upper(file_name);
    char* f = "";
    file_write(file_name, f, strlen(f));
}

void remove(command cmd) {
    if (cmd.argc != 2) {
        print("Incorrect Usage...\n");
        return;
    }

    // get second argument
    char* file_name = cmd.argv;
    file_name += strlen(cmd.argv) + 1;
    file_name = to_upper(file_name);
    file_delete(file_name);

}

void cat(command cmd) {

    if (cmd.argc != 2) {
        print("Incorrect Usage...\n");
        return;
    }

    // get second argument
    char* file_name = cmd.argv;
    file_name += strlen(cmd.argv) + 1;

    char* buffer = command_head;

    DIR_ENTRY* file = file_find(file_name);

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

// void edit_line(char* buffer, int current_line) {
//     for (int i = 0; i < current_line; i++) {
//         buffer = strchr(buffer, '\n') + 1;
//     }

//     char* line_end = strchr(buffer, '\n');
//     char saved[1024] = { 0 };
//     strcpy(saved, line_end);
//     scanf("%s", buffer);
//     strcpy(buffer + strlen(buffer), saved);
// }

void edit(command cmd) {

    if (cmd.argc != 2) {
        print("Incorrect Usage...\n");
        return;
    }

    // get second argument
    char* file_name = cmd.argv;
    file_name += strlen(cmd.argv) + 1;

    char file_buffer[1024] = {0};

    file_read(file_name, file_buffer);

    while(true) {

        clear();
        print(file_name);
        print("\n===============\n");

        size_t lines_count = 0;
        char* fb = file_buffer;
        while(true) {

            print_int(lines_count);
            print(" | ");
            print_line(fb);

            fb = strchr(fb, '\n');

            if (fb == NULL) {
                break;
            }
            
            fb++;
            lines_count++;

        }

        size_t blank = VGA_HEIGHT - lines_count - 3;
        for (int i=0;i<blank;i++){
            print("\n");
        }

        print("EDIT>");

        char input_buffer[64] = {0};
        size_t i = 0;
        while (true) {
            char in = keyboard_getinput();

            // enter to finish command
            if (in == '\n') {
                break;
            }

            if (!in) {
                continue;
            }

            // backspace
            if (in == '\b') {

                if (i == 0) {
                    continue;
                } 

                i--;
                input_buffer[i] = 0;
                printc(in);
                continue;
            }

            input_buffer[i] = in;
            i++;

            printc(in);

        }

        // exit if q
        if (strcmp(input_buffer, "q")) {
            break;
        }

        // write buffer to disk if w
        if (strcmp(input_buffer, "w")) {
            file_write(file_name, file_buffer, strlen(file_buffer));
        }

        size_t input_len = strlen(input_buffer);
        size_t tokens = strtok(input_buffer, ':', true);

        // incorrect format
        if (tokens != 2) {
            continue;
        }

        if (input_buffer[0] == 'a') {
            fb = file_buffer;
            fb += strlen(fb);
            *fb = '\n';
            fb++;
            
            char* updated_line = input_buffer + strlen(input_buffer) + 1;
            updated_line[strlen(updated_line)] = '\0';
    
            strcpy(updated_line, fb, strlen(updated_line) + 1);
            continue;
        }

        if (!is_int(input_buffer[0])) {
            continue;
        }

        size_t line = input_buffer[0] - '0';
        
        fb = file_buffer;
        char swap_buffer[1024] = {0};
        char* sb = swap_buffer;
        for (int i=0;i<line;i++) {
            strcpy(fb, sb, strlen(fb) + 1);
            fb = strchr(fb, '\n') + 1;
            sb = strchr(sb, '\n') + 1;
        }

        char* updated_line = input_buffer + strlen(input_buffer) + 1;
        strcpy(updated_line, sb, strlen(updated_line));

        fb = strchr(fb, '\n') + 1;
        sb = strchr(sb, '\n') + 1;

        for (int i=line+1;i<lines_count;i++){
            strcpy(fb, sb, strlen(fb) + 1);
            fb = strchr(fb, '\n') + 1;
            sb = strchr(sb, '\n') + 1;
        }
        
        strcpy(swap_buffer, file_buffer, 1024);
    
    }

    print("\n");

    return;

    // int current_line = 0;
    // scanf("%d", &current_line);
    // edit_line(buffer, current_line);

    // f = fopen(argv[1], "w");
    // fwrite(buffer, strlen(buffer), 1, f);

    // fclose(f);
}
