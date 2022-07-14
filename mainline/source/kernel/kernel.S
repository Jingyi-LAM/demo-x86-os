.globl  kstack
.globl  _start
.globl  desc_gdt
.globl  sel_syscode
.globl  sel_sysdata

############################### Native kernel
.section        .text
.code32 

_start:
        lgdt    gdtmeta

        movw    sel_sysdata,    %ax
        movw    %ax,            %ds
        movw    %ax,            %es
        movw    %ax,            %ss
        movw    %ax,            %fs
        movl    $kstack,        %esp 

        # Initialize interrupt vector table and test
        call    idt_init
        lidt    idt_meta
        call    chip8259a_init

        call    process_pre_init
        call    ipc_init
        call    create_task
        call	keyboard_init

        movl    ptr_proc_run,   %esp
        lldt    72(%esp)
        lea     72(%esp),       %eax
        movl    $tss,           %ebx
        movl    %eax,           4(%ebx)

        popl    %gs
        popl    %fs
        popl    %es
        popl    %ds
        popa
        addl    $4,             %esp
        iret
############################### Native kernel end



############################### Kernel stack
.section        .data

        . =     . + 2048
kstack:
############################### Kernel stack end



############################### Global descriptor table
.section        .data

# GDT Pointer
gdtmeta:
        .word   2048
        .long   desc_gdt

# desc_syscode and desc_sysdata are temporary descriptors
desc_gdt:
        .long   0, 0
desc_syscode:
        .word	0b1111111111111111
        .word	0b0000000000000000
        .word	0b1001101000000000
        .word	0b0000000011001111
desc_sysdata:
        .word	0b1111111111111111
        .word	0b0000000000000000
        .word	0b1001001000000000
        .word	0b0000000011001111

# Support up to 256 descriptors
        . =     . + 2048 - 24

sel_syscode:
        .long   desc_syscode - desc_gdt
sel_sysdata:
        .long   desc_sysdata - desc_gdt
############################### Global descriptor table end
