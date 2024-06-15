asm=nasm
CC=i686-elf-gcc
linker=i686-elf-ld

all: boot kernel final

boot: src/boot.asm
	$(asm) -f bin src/boot.asm -o build/boot.bin

kernel: src/kernel_entry.asm src/kernel.c
	$(asm) src/kernel_entry.asm -f elf -o build/kernel_entry.o
	$(CC) -ffreestanding -m32 -g -o build/kernel.o -c src/kernel.c
	$(linker) -o build/full_kernel.bin -Ttext 0x1000 build/kernel_entry.o build/kernel.o --oformat binary

final: build/full_kernel.bin build/boot.bin
	cat build/boot.bin build/full_kernel.bin > build/OS.bin
	truncate -s 1440k build/OS.bin

clean:
	rm build/*
