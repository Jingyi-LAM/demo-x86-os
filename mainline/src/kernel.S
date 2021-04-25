.code32 
.section        .text
        jmp     _start

.section        .data
############################### global descriptor table
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
# new descriptor here
desc_tss:
        .long   0, 0
desc_ldt:
        .long   0, 0

gdtptr:
        .word   . - desc_gdt - 1
        .long   desc_gdt

.globl  desc_tss
############################### global descriptor table end

############################### kernel stack
        . =     . + 2048
kstack:
############################### kernel stack end

sel_syscode	= desc_syscode	- desc_gdt
sel_sysdata	= desc_sysdata	- desc_gdt
sel_tss         = desc_tss      - desc_gdt


.globl          _start
.globl          kstack
.code32
.section        .text
_start:
        lgdt    gdtptr

        movw    $sel_sysdata,   %ax
        movw    %ax,            %ds
        movw    %ax,            %es
        movw    %ax,            %ss
        movw    %ax,            %fs
        movl    $kstack,        %esp 

        sgdt    gdt_base_addr
        call    gdt_init
        lgdt    gdt_base_addr

        # Initialize interrupt vector table and test
        call    idt_init
        lidt    idt_base_addr
        call    chip8259a_init

        # Initialize tss
        pushl   %esp
        pushl   $sel_sysdata
        call    tss_init
        popl    %ebx 
        popl    %ebx 
        ltr     %ax

        call    process_pre_init
        call    ipc_init
        call    create_task
        call	keyboard_init
	cli
	
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


#        xorl    %eax,           %eax
#        call    ldt_init
#        lldt    %ax
#
#        movw    $15,            %ax
#        movw    %ax,            %ds
#        movw    %ax,            %es
#        movw    %ax,            %fs
#        movw    %ax,            %gs
#
#        push    $15
#        push    $0x30400
#        push    $0
#        push    $7
#        push    $demo_process1
#        iret
        
