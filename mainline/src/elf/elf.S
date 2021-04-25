.globl          elf_load_file

.code32
.section        .text
# ----------------------------------------
#   input:
#       push (The memory address of the ELF file)
# ----------------------------------------
elf_load_file:
        pushl   %ebp
        movl    %esp,           %ebp
        
        pushl   %eax
        pushl   %ebx      
        pushl   %ecx

        xorl    %ecx,           %ecx

        movl    8(%ebp),        %ebx
        movw    44(%ebx),       %cx
        movl    28(%ebx),       %esi
        addl    %ebx,           %esi
.start_loading:
        cmpl    $0,             %ecx
        jz      .finish_loading

        movl    (%esi),         %eax
        cmpl    $0,             %eax
        jz      .null

        # from
        movl    4(%esi),        %eax
        addl    %ebx,           %eax
        pushl   %eax
        # to
        pushl   8(%esi)
        # size
        pushl   16(%esi)
        call    elf_memcpy

.null:
        addl    $32,            %esi
        decl    %ecx
        jnz     .start_loading
.finish_loading:
        popl    %ecx
        popl    %ebx
        popl    %eax
        popl    %ebp
        ret     $4



# ----------------------------------------
#   input:
#       push src ptr
#       push dest ptr
#       push size
# ----------------------------------------
elf_memcpy:
        pushl   %ebp
        movl    %esp,           %ebp
        
        pushl   %esi
        pushl   %edi
        pushl   %ecx

        movl    16(%ebp),       %esi
        movl    12(%ebp),       %edi
        movl    8(%ebp),        %ecx

.continue_copying:
        cmpl    $0,             %ecx
        jz      .finish_copying
        
        movb    %ds:(%esi),     %al
        movb    %al,            %es:(%edi)
        incl    %esi
        incl    %edi
        decl    %ecx
        jmp     .continue_copying
.finish_copying:
        movl    12(%ebp),       %eax
        popl    %ecx
        popl    %edi
        popl    %esi
        popl    %ebp
        ret     $12

