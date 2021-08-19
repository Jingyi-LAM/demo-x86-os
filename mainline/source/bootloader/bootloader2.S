.globl  _start
.code16
.section        .text
        jmp     _start

# GDT & LDT ------------------------------------------------
# [15,  0]  segment limit1
# [39, 16]  segment base address1
# [55, 40]  G | D/B | 0 | AVL | segment limit2(4 bits) | P | DPL(2 bits) | S | TYPE(4 bits)
# [63, 56]  segment base address2
# ----------------------------------------------------------
.section        .data
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
desc_gsdata:
	.word	0b1111111111111111
	.word	0b0000000000000000
	.word	0b1111001000000000
	.word	0b0000000011001111
desc_pagedir:
	.long	0, 0
desc_pagetbl:
	.long	0, 0
desc_tmp:
	.long	0, 0
gdtptr:
        .word   . - desc_gdt - 1
        .long   desc_gdt

sel_syscode	= desc_syscode	- desc_gdt
sel_sysdata	= desc_sysdata	- desc_gdt
sel_pagedir	= desc_pagedir	- desc_gdt
sel_pagetbl	= desc_pagetbl	- desc_gdt
sel_tmp         = desc_tmp     - desc_gdt

kernel_name:    .ascii  "KN      BIN"
bl_stack:       . = . + 2048


.code16
.section        .text
_start:
        movw    %cs,            %ax
        movw    %ax,            %ds
        movw    %ax,            %es
        movw    %ax,            %fs
        movw    %ax,            %ss
        movw    $0,             %sp


        # search for target file in fat12
        xor     %edi,           %edi
        movw    %ds,            %di
        shl     $4,             %edi
        movw    $19,            %ax
        movw    $14,            %bx
        movl    $kernel_name,   %esi
        subl    %edi,           %esi
        call    fat12_search4file
        cmpw    $0,             %ax
        # dead loop when the target file was not found
        jz      .

        # now ax stores the clus number of the target file
        movw    $0x8000,        %bx
        movw    %bx,            %es
        movw    $0,             %bx
        movw    %bx,            %di
        call    fat12_copy_to_mem

	# page dir init
	movl	$0x200000,	%eax
	movl	$4096,		%ebx
	movl	$0x0092,	%ecx
	movl    $desc_pagedir,	%edx
	call	write_desc

	# page table init
	movl	$0x201000,	%eax
	movl	$1024,		%ebx
	movl	$0x8092,	%ecx
	movl	$desc_pagetbl,	%edx
	call	write_desc

        movl    $syscode32,     %eax
        movl    $len_syscode32, %ebx
        movl    $0xc09a,        %ecx
        movl    $desc_tmp,      %edx
        call    write_desc

        movl    $gdtptr,        %eax
        xor     %ebx,           %ebx
        movw    %ds,            %bx
        shl     $4,             %ebx
        subl    %ebx,           %eax
        cli
        lgdt    %ds:(%eax)

        in      $92,            %al
        orb     $2,             %al
        out     %al,            $92
        
        movl    %cr0,           %eax
        orl     $0x1,           %eax
        movl    %eax,           %cr0

        ljmp    $sel_tmp,       $0


.code32
# ring0 here
syscode32:
	# call	write_page_trans

        movw    $sel_sysdata,   %ax
        movw    %ax,            %ds
	movw	%ax,		%es
        movl    $0,             %edi
        movb    $0xc,           %ah
        movb    $'5',           %al
        movw    %ax,            0xb8002(%edi)

        movw    $sel_sysdata,   %ax
        movw    %ax,            %ds
        movl    $0x80000,       %eax
        pushl   %eax
        call    elf_load_file

        ljmp    $sel_syscode,   $0x30400
	jmp	.
len_syscode32   = . - syscode32


write_page_trans:
	movw	$sel_pagedir,	%ax
	movw	%ax,		%ds	
	xor	%edi,		%edi
	movl	$1024,		%ecx
	movl	$0x201000,	%eax
	addl	$7,		%eax
1:
	movl	%eax,		(%edi)
	addl	$4096,		%eax
	addl	$4,		%edi
	loop	1b
	
	movw	$sel_pagetbl,	%ax
        movw    %ax,            %ds
	movl	$1048576,	%ecx
	xor	%edi,		%edi
	xor	%eax,		%eax
	addl	$7,		%eax
2:
	movl	%eax,		(%edi)
	addl	$4096,		%eax
	addl	$4,		%edi
	loop	2b

	movl	$0x200000,	%eax
	movl	%eax,		%cr3
	movl	%cr0,		%eax
	orl	$0x80000000,	%eax
	movl	%eax,		%cr0
	nop
	nop
	nop
	ret


.code16
# fill in base address and limit length --------------------
# input:
#       %eax    base address
#       %ebx    limit length
#       %ecx    attribution
#       %edx    target descriptor address
# return:
#       none
# ----------------------------------------------------------
write_desc:
        pushl   %eax
        xor     %eax,           %eax
        movw    %ds,            %ax
        shl     $4,             %eax
        subl    %eax,           %edx
        popl    %eax

        # base address
        movw    %ax,            2(%edx)
        shr     $16,            %eax
        movb    %al,            4(%edx)
        movb    %ah,            7(%edx)

        # limit/attr
        movw    %bx,            (%edx)
        shr     $16,            %ebx
        shl     $8,             %bx
        orw     %bx,            %cx
        movw    %cx,            5(%edx)

        ret

# fill in base address and limit length --------------------
# input:
#       %eax    selector
#       %ebx    offset
#       %ecx    attribution
#       %edx    target gate address
# return:
#       none
# ----------------------------------------------------------
write_gate:
        pushl   %eax
        xor     %eax,           %eax
        movw    %ds,            %ax
        shl     $4,             %eax
        subl    %eax,           %edx
        popl    %eax

	movw	%ax,		2(%edx)
	movw	%bx,		(%edx)		
	shr	$16,		%ebx
	movw	%bx,		6(%edx)
	movw	%cx,		4(%edx)
	ret



