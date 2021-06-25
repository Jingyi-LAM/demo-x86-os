ST_ENTRY= 0x7c00
BL_ENTRY= 0x90000
KN_ENTRY= 0x30400

NAS	= nasm
AS	= as
CC	= gcc
LD	= ld
CC	= gcc

#-----------------------------
# compile flags
BIT32	= --32
CFLAGS	= -m32 -fno-stack-protector
INCLUDE = -I io/ -I process/ -I debug/ -I common/ -I interrupt/ -I ./ -I heap/ -I user_app/

#-----------------------------
# link flags
ELF	= -m elf_i386
BINARY	= --oformat binary

.PHONY	: all clean
all	: bl1.bin bl2.bin kn.bin

# bootloader
bl1.bin	: boot/bootloader1.asm
	$(NAS) $^ -o $@
bl2.o	: boot/bootloader2.S
	$(AS) $^ -o $@
fat12.o	: fat12/fat12.S
	$(AS) $^ -o $@
elf.o   : elf/elf.S
	$(AS) $^ -o $@ 
bl2.bin : bl2.o fat12.o elf.o
	$(LD) -Ttext $(BL_ENTRY) $(BINARY) $^ -o $@

# kernel
kn_S.o	: kernel.S
	$(AS) $(BIT32) $^ -o $@
kn_c.o	: kernel.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@

common.o: common/common.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@
string.o: common/string.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@
int_S.o	: interrupt/interrupt.S
	$(AS) $(BIT32) $^  -o $@
int_c.o : interrupt/interrupt.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@
debug.o : debug/debug.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@

proc.o : process/process.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@
heap.o : heap/heap.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@
ipc.o  : process/ipc.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@
task.o : user_app/task.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@
kb.o    : io/keyboard.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@
io.o    : io/io.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@
hd.o    : io/hd.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@
tty.o   : io/tty.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@
kn.bin  : kn_S.o kn_c.o common.o int_S.o int_c.o proc.o debug.o task.o ipc.o kb.o tty.o heap.o string.o io.o hd.o
	$(LD) $(ELF) -Ttext $(KN_ENTRY) $^ -o $@

clean   :
	rm -f *.o *.bin



