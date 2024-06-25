ASM=nasm
ASMFLAGS=-f elf
CC=i686-elf-gcc
CCFLAGS=-ffreestanding -nostdlib
LD=i686-elf-ld
LFLAGS=-T linker.ld
SRC=src
BUILD=build

C_SRC := $(shell find -wholename './src/kernel/*.c')
C_OBJ := $(patsubst ./src/kernel/%, ./build/%, $(patsubst %.c,%.o, $(C_SRC)))
ASM_SRC := $(shell find -wholename './src/kernel/*.asm')
ASM_OBJ := $(patsubst ./src/kernel/%, ./build/%, $(patsubst %.asm,%.o, $(ASM_SRC)))

all: floppy

# compiling all the c source files into object files
$(BUILD)/%.o: $(SRC)/kernel/%.c
	@$(CC) $(CCFLAGS) -o $@ -c $<
	@echo "Compiled" $<

# assembling all the asm source files into object files
$(BUILD)/%.o: $(SRC)/kernel/%.asm
	@$(ASM) $(ASMFLAGS) -o $@ $<
	@echo "Assembled" $<

# building the stage1 binary
$(BUILD)/bootloader/stage1.bin: $(SRC)/bootloader/stage1.asm
	@$(ASM) -f bin $^ -o $@
	@echo "Built stage1.bin"

# building the stage2 binary
$(BUILD)/bootloader/stage2.bin: $(SRC)/bootloader/stage2.asm
	@$(ASM) -f bin $^ -o $@
	@echo "Built stage2.bin"

# building the complete bootloader binary
$(BUILD)/bootloader/bootloader.bin: $(BUILD)/bootloader/stage1.bin $(BUILD)/bootloader/stage2.bin
	@cat $^ > $@
	@echo "Built bootloader.bin\n============================"

# building the complete kernel binary
$(BUILD)/kernel.bin: $(ASM_OBJ) $(C_OBJ)
	@$(LD) -o $@ $^ $(LFLAGS)
	@echo "Built kernel.bin\n============================"

# creating a 1440 KiB floppy img with the bootloader and the kernel binaries
floppy: $(BUILD)/bootloader/bootloader.bin $(BUILD)/kernel.bin
	@dd if=/dev/zero of=$(BUILD)/floppy.img bs=512 count=2880 status=progress 2>/dev/null
	@mkfs.fat -F 12 -n "DOS" -R 2 $(BUILD)/floppy.img >/dev/null
	@dd if=$(BUILD)/bootloader/bootloader.bin of=$(BUILD)/floppy.img conv=notrunc status=progress 2>/dev/null
	@mcopy -i $(BUILD)/floppy.img $(BUILD)/kernel.bin "::kernel.bin"
	@mcopy -i $(BUILD)/floppy.img files/test.txt "::test.txt"
	@echo "Created floppy.img"

clean:
	@rm -r $(BUILD)/*
	@mkdir $(BUILD)/bootloader
	@echo "Cleared $(BUILD)/"
	