#Makefile

#Entry point of OS kernel
ENTRYPOINT = 0x30400

#Offset of entry point in kernel file
ENTRYOFFSET = 0x400

#Programs,flags,etc.
ASM = nasm
DASM = ndisasm
CC = gcc
LD = ld
ASMBFLAGS = -I boot/include/
ASMKFLAGS = -I include/ -f elf
CFLAGS = -I include/ -c -fno-builtin -m32
LDFLAGS = -s -Ttext $(ENTRYPOINT) -m elf_i386
DASMFLAGS = -u -o $(ENTRYPOINT) -e $(ENTRYOFFSET)

#This Program
OSBOOT = bin/boot.bin bin/loader.bin
OSKERNEL = bin/kernel.bin
OBJS = bin/obj/kernel.o bin/obj/start.o bin/obj/kliba.o bin/obj/string.o \
       bin/obj/i8259.o bin/obj/global.o bin/obj/protect.o bin/obj/klib.o \
       bin/obj/main.o bin/obj/clock.o bin/obj/syscall.o bin/obj/proc.o \
       bin/obj/keyboard.o bin/obj/tty.o bin/obj/console.o \
       bin/obj/printf.o bin/obj/vsprintf.o
DASMOUTPUT = diasm/kernel.bin.asm

#All Phony Targets
.PHONY : everything final image clean realclean disasm all buildimg debug debugUI

#Default starting position

everything : $(OSBOOT) $(OSKERNEL)

all : realclean everything

final : all clean

image : all buildimg

clean :
	rm -f $(OBJS)

realclean :
	rm -f $(OBJS) $(OSBOOT) $(OSKERNEL)

disasm :
	$(DASM) $(DASMFLAGS) $(OSKERNEL) > $ (DASMOUTPUT)

building :
	dd if=bin/boot.bin of=bin/debug/a.img bs=512 count=1 conv=notrunc
	sudo mount -o loop bin/debug/a.img /mnt/floppy
	sudo cp -fv bin/loader.bin /mnt/floppy
	sudo cp -fv bin/kernel.bin /mnt/floppy
	sudo umount /mnt/floppy

debug :
	cd bin/debug
	bochs -q

bin/boot.bin : boot/boot.asm
	$(ASM) $(ASMBFLAGS) -o $@ $<

bin/loader.bin : boot/loader.asm boot/include/lib.inc boot/include/pm.inc boot/include/ReadFileInRoot.asm
	$(ASM) $(ASMBFLAGS) -o $@ $<

$(OSKERNEL) : $(OBJS)
	$(LD) $(LDFLAGS) -o $(OSKERNEL) $(OBJS)

bin/obj/kernel.o : kernel/kernel.asm include/sconst.inc
	$(ASM) $(ASMKFLAGS) -o $@ $<

bin/obj/syscall.o : kernel/syscall.asm include/sconst.inc
	$(ASM) $(ASMKFLAGS) -o $@ $<

bin/obj/start.o : kernel/start.c include/type.h include/const.h include/protect.h include/proto.h include/string.h include/proc.h include/global.h
	$(CC) $(CFLAGS) -o $@ $<

bin/obj/kliba.o : lib/kliba.asm include/global.h include/sconst.inc
	$(ASM) $(ASMKFLAGS) -o $@ $<

bin/obj/string.o : lib/string.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

bin/obj/i8259.o : kernel/i8259.c include/type.h include/const.h include/protect.h include/proto.h include/proc.h include/global.h
	$(CC) $(CFLAGS) -o $@ $<

bin/obj/global.o : kernel/global.c include/global.h include/type.h include/const.h include/protect.h include/proto.h include/proc.h
	$(CC) $(CFLAGS) -o $@ $<

bin/obj/protect.o : kernel/protect.c include/const.h include/protect.h include/type.h include/proc.h include/global.h include/proto.h
	$(CC) $(CFLAGS) -o $@ $<

bin/obj/klib.o : lib/klib.c include/type.h include/const.h include/protect.h include/proto.h include/string.h include/proc.h include/global.h
	$(CC) $(CFLAGS) -fno-stack-protector -o $@ $<

bin/obj/main.o : kernel/main.c include/type.h include/const.h include/protect.h include/proto.h include/string.h include/proc.h include/global.h
	$(CC) $(CFLAGS) -o $@ $<
bin/obj/clock.o : kernel/clock.c include/type.h include/const.h include/protect.h include/proto.h include/string.h include/proc.h include/global.h
	$(CC) $(CFLAGS) -o $@ $<

bin/obj/proc.o : kernel/proc.c include/type.h include/const.h include/protect.h include/proto.h include/string.h include/proc.h include/global.h
	$(CC) $(CFLAGS) -o $@ $<

bin/obj/keyboard.o : kernel/keyboard.c include/type.h include/const.h include/protect.h include/proto.h include/string.h include/proc.h include/global.h include/keyboard.h include/keymap.h
	$(CC) $(CFLAGS) -fno-stack-protector -o $@ $<

bin/obj/tty.o : kernel/tty.c include/type.h include/const.h include/protect.h include/proto.h include/string.h include/proc.h include/global.h include/keyboard.h include/tty.h include/console.h
	$(CC) $(CFLAGS) -o $@ $<

bin/obj/console.o : kernel/console.c include/type.h include/const.h include/protect.h include/proto.h include/string.h include/proc.h include/global.h include/keyboard.h include/tty.h include/console.h
	$(CC) $(CFLAGS) -o $@ $<

bin/obj/printf.o : kernel/printf.c include/type.h include/const.h
	$(CC) $(CFLAGS) -fno-stack-protector -o $@ $<

bin/obj/vsprintf.o : kernel/vsprintf.c include/type.h include/const.h include/string.h
	$(CC) $(CFLAGS) -fno-stack-protector -o $@ $<

bin/obj/misc.o : lib/misc.c include/type.h include/const.h include/string.h include/protect.h include/proc.h include/tty.h include/console.h include/global.h include/keyboard.h include/proto.h
	$(CC) $(CFLAGS) -o $@ $<

