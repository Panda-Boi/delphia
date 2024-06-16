#include <stdio.h>

int main(int argc, char* argv[]) {

    if (argc < 3) {
        printf("Usage: %s <Disk Image> <File Name>\n", argv[0]);
        return 1;
    }

    FILE* disk = fopen(argv[1], "rb");
    char* file_name = argv[2];



    return 0;

}