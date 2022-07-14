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
CFLAGS	= -m32 -fno-stack-protector -fno-builtin
INCLUDE = -I io/ -I process/ -I common/ -I interrupt/ -I ./ -I heap/ -I app/ -I kernel/

#-----------------------------
# link flags
ELF	= -m elf_i386
BINARY	= --oformat binary

.PHONY	: all clean
all	: bl1.bin bl2.bin kn.bin

# bootloader
bl1.bin	: bootloader/bootloader1.asm
	$(NAS) $^ -o $@
bl2.o	: bootloader/bootloader2.S
	$(AS) $^ -o $@
fat12.o	: asm_tool/fat12.S
	$(AS) $^ -o $@
elf.o   : asm_tool/elf.S
	$(AS) $^ -o $@ 
bl2.bin : bl2.o fat12.o elf.o
	$(LD) -Ttext $(BL_ENTRY) $(BINARY) $^ -o $@

# kernel
kn_S.o	: kernel/kernel.S
	$(AS) $(BIT32) $^ -o $@
kn_c.o	: kernel/kernel.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@

desc.o  : common/descriptor.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@
memory.o: common/memory.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@
string.o: common/string.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@
int_S.o	: interrupt/interrupt.S
	$(AS) $(BIT32) $^  -o $@
int_c.o : interrupt/interrupt.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@
debug.o : common/debug.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@
log.o   : common/log.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@

proc.o : process/process.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@
heap.o : heap/heap.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@
kheap.o: heap/kheap.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@
ipc.o  : process/ipc.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@
task.o : app/task.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@
kb.o    : io/keyboard.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@
io.o    : io/io.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@
hd.o    : io/hd.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@
fs.o    : io/fs.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@
tty.o   : io/tty.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@
kn.bin  : kn_S.o kn_c.o desc.o int_S.o int_c.o proc.o debug.o task.o ipc.o kb.o tty.o heap.o string.o io.o hd.o fs.o memory.o log.o kheap.o
	$(LD) $(ELF) -Ttext $(KN_ENTRY) $^ -o $@

clean   :
	rm -f *.o *.bin



