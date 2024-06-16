asm=nasm
CC=i686-elf-gcc
linker=i686-elf-ld
src=src
build=build

all: bootloader kernel binary floppy

bootloader: src/boot.asm
	$(asm) -f bin src/boot.asm -o build/bootloader.bin

kernel: src/kernel_entry.asm src/kernel.c
	$(asm) src/kernel_entry.asm -f elf -o build/kernel_entry.o
	$(CC) -ffreestanding -m32 -g -o build/kernel.o -c src/kernel.c
	$(linker) -o build/kernel.bin -Ttext 0x1000 build/kernel_entry.o build/kernel.o --oformat binary

binary: build/kernel.bin build/boot.bin
	cat build/bootloader.bin build/kernel.bin > build/OS.bin
	truncate -s 1440k build/OS.bin

# creating a 1440 KiB floppy img
floppy: $(build)/kernel.bin $(build)/bootloader.bin
	dd if=/dev/zero of=$(build)/floppy.img bs=512 count=2880
	mkfs.fat -F 12 -n "DOS" $(build)/floppy.img
	dd if=$(build)/bootloader.bin of=$(build)/floppy.img conv=notrunc
	mcopy -i $(build)/floppy.img $(build)/kernel.bin "::kernel.bin"

clean:
	rm build/*