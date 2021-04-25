.section        .text
.code32

.macro  exception               irq_no
        pushl   \irq_no
        call    exception_handler
        addl    $4,             %esp
        hlt
.endm

.macro  interrupt       irq_no, irq_type
#        subl    $4,             %esp
#        pusha
#        pushl   %ds
#        pushl   %es
#        pushl   %fs
#        pushl   %gs
#
#        movl    $kstack,        %esp        
#        movl    \irq_no,        %eax
#        call    *handler_table(, %eax, 4)        
#        movl    ptr_proc_run,   %esp
#
#        popl    %gs
#        popl    %fs
#        popl    %es
#        popl    %ds
#        popa
#        addl    $4,             %esp
#        iret


#######################################
## Timer test in ring0
#        movl    $0xb8010,       %edi
#        movb    $0xc,           %ah
#        movb    %ah,            1(%edi)
#        incb    (%edi)
#
#        movb    $0x20,          %al
#        outb    %al,            $0x20
#        iret
#######################################

        subl    $4,             %esp
        pusha
        pushl   %ds
        pushl   %es
        pushl   %fs
        pushl   %gs

        movw    %ss,            %dx
        movw    %dx,            %ds
        movw    %dx,            %es
        
        
        movb    \irq_type,      %al
        cmpb    $0,             %al
        je      1f
        jne     2f


1:	
        in      $0x21,          %al     # ignore current interrupt (master)
        movb    \irq_no,        %cl
        subb    $32,            %cl
        movb    $1,             %bl
        shl     %cl,            %bl
        or      %bl,            %al
        out     %al,            $0x21

2:
        in      $0xa1,          %al     # ignore current interrupt (slave)
        movb    \irq_no,        %cl
        subb    $40,            %cl
        movb    $1,             %bl
        shl     %cl,            %bl
        orb     %bl,            %al
        out     %al,            $0xa1
        
        movb    $0x20,          %al
        out     %al,            $0x20
        nop
        out     %al,            $0xa0

3:
	incl    k_reenter
	movl    k_reenter,      %eax
	cmp     $0,             %eax
	jne     7f

        movl    $kstack,        %esp
        sti
        movl    \irq_no,        %esi
        call    *handler_table(, %esi, 4)
	cli

        movb    $0x20,          %al
        outb    %al,            $0x20
        
        movb    \irq_type,      %al
        cmp     $0,             %al
        je      4f
        jne     5f

4:                                      # master
        in      $0x21,          %al				
        movb    \irq_no,        %cl
        subb    $32,            %cl
        movb    $1,             %bl
        shl     %cl,            %bl
        not     %bl
        and     %bl,            %al
        out     %al,            $0x21
        jmp     6f

5:                                      # slave
        in      $0xa1,          %al
        movb    \irq_no,        %cl
        sub     $40,            %cl
        movb    $1,             %bl
        shl     %cl,            %bl
        not     %bl
        and     %bl,            %al
        out     %al,            $0xa1

6:
        movl    ptr_proc_run,   %esp
7:
	decl    k_reenter
        pop     %gs
        pop     %fs
        pop     %es
        pop     %ds
        popa
        addl    $4,             %esp
        iret

.endm

.globl          excp000
.globl          excp001
.globl          excp002
.globl          excp003
.globl          excp004
.globl          excp005
.globl          excp006
.globl          excp007
.globl          excp008
.globl          excp009
.globl          excp010
.globl          excp011
.globl          excp012
.globl          excp013
.globl          excp014
.globl          excp015
.globl          excp016
.globl          excp017
.globl          excp018
.globl          excp019
.globl          excp020
.globl          excp021
.globl          excp022
.globl          excp023
.globl          excp024
.globl          excp025
.globl          excp026
.globl          excp027
.globl          excp028
.globl          excp029
.globl          excp030
.globl          excp031
.globl          hwint00
.globl          hwint01
.globl          hwint02
.globl          hwint03
.globl          hwint04
.globl          hwint05
.globl          hwint06
.globl          hwint07
.globl          hwint08
.globl          hwint09
.globl          hwint10
.globl          hwint11
.globl          hwint12
.globl          hwint13
.globl          hwint14
.globl          hwint15
.globl          syscall

excp000:        exception       $0x00
excp001:        exception       $0x01
excp002:        exception       $0x02
excp003:        exception       $0x03
excp004:        exception       $0x04
excp005:        exception       $0x05
excp006:        exception       $0x06
excp007:        exception       $0x07
excp008:        exception       $0x08
excp009:        exception       $0x09
excp010:        exception       $0x0a
excp011:        exception       $0x0b
excp012:        exception       $0x0c
excp013:        exception       $0x0d
excp014:        exception       $0x0e
excp015:        exception       $0x0f
excp016:        exception       $0x10
excp017:        exception       $0x11
excp018:        exception       $0x12
excp019:        exception       $0x13
excp020:        exception       $0x14
excp021:        exception       $0x15
excp022:        exception       $0x16
excp023:        exception       $0x17
excp024:        exception       $0x18
excp025:        exception       $0x19
excp026:        exception       $0x1a
excp027:        exception       $0x1b
excp028:        exception       $0x1c
excp029:        exception       $0x1d
excp030:        exception       $0x1e
excp031:        exception       $0x1f
hwint00:        interrupt       $0x20,  $0
hwint01:        interrupt       $0x21,  $0
hwint02:        interrupt       $0x22,  $0
hwint03:        interrupt       $0x23,  $0
hwint04:        interrupt       $0x24,  $0
hwint05:        interrupt       $0x25,  $0
hwint06:        interrupt       $0x26,  $0
hwint07:        interrupt       $0x27,  $0
hwint08:        interrupt       $0x28,  $1
hwint09:        interrupt       $0x29,  $1
hwint10:        interrupt       $0x2a,  $1
hwint11:        interrupt       $0x2b,  $1
hwint12:        interrupt       $0x2c,  $1
hwint13:        interrupt       $0x2d,  $1
hwint14:        interrupt       $0x2e,  $1
hwint15:        interrupt       $0x2f,  $1


syscall:
        subl    $4,             %esp
        pusha
        pushl   %ds
        pushl   %es
        pushl   %fs
        pushl   %gs
        
        movl    %edx,           %esi
        movw    %ss,            %dx
        movw    %dx,            %ds
        movw    %dx,            %es
        movl    %esi,           %edx
        
        movl    %esp,           %esi
        
        incl    k_reenter
        cmpl    $0,             k_reenter
        jne     1f
        
        movl    $kstack,        %esp
        
        push    %esi

        push    %edx
        push    %ecx
        push    %ebx
        push    %eax
        movl    $100,           %edi
        call    *handler_table(, %edi, 4)

        addl    $16,            %esp
        popl    %esi
        
        movl    %eax,           44(%esi) # return value of syscall handler function
        movl    ptr_proc_run,   %esp
1:
        decl    k_reenter
        popl    %gs
        popl    %fs
        popl    %es
        popl    %ds
        popa
        addl    $4,             %esp
        iret
 
