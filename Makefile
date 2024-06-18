asm=nasm
CC=i686-elf-gcc
linker=i686-elf-ld
src=src
build=build

all: stage1 stage2 bootloader kernel binary floppy

stage1: src/bootloader/stage1.asm
	$(asm) -f bin $? -o build/bootloader/stage1.bin

stage2: src/bootloader/stage2.asm
	$(asm) -f bin $? -o build/bootloader/stage2.bin

bootloader: $(build)/bootloader/stage1.bin $(build)/bootloader/stage2.bin
	cat $? > $(build)/bootloader/bootloader.bin

kernel: src/kernel_entry.asm
# $(asm) $? -f bin -o build/kernel_entry.bin
	$(asm) src/kernel_entry.asm -f elf -o build/kernel_entry.o
	$(CC) -ffreestanding -nostdlib -o build/kernel.o -c src/kernel.c
	$(linker) -o build/kernel.bin build/kernel_entry.o build/kernel.o -T linker.ld

binary: build/kernel.bin build/bootloader/bootloader.bin
	cat build/bootloader/bootloader.bin build/kernel.bin > build/OS.bin
	truncate -s 1440k build/OS.bin

# creating a 1440 KiB floppy img
floppy: $(build)/kernel.bin $(build)/bootloader/bootloader.bin
	dd if=/dev/zero of=$(build)/floppy.img bs=512 count=2880
	mkfs.fat -F 12 -n "DOS" -R 2 $(build)/floppy.img
	dd if=$(build)/bootloader/bootloader.bin of=$(build)/floppy.img conv=notrunc
	mcopy -i $(build)/floppy.img $(build)/kernel.bin "::kernel.bin"

clean:
	rm build/*