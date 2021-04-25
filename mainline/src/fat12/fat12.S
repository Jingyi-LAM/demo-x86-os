.globl  fat12_search4file
.globl  fat12_copy_to_mem

.data
global_buffer:                  . = . + 2048

.code16
# ------------------------------------
# input:
#       ax          The starting sector number to be searched
#       bx          Number of sectors to be searched
#       cx          Segment Base Address Loading ROOT DIR Entries
#       dx          The offset of loading ROOT DIR entries
#       ds:si       File name
# ------------------------------------
fat12_search4file:
        pushw   %bp
        pushw   %ax
        pushw   %bx
        pushw   %ds 
        pushw   %ds
        pushw   %dx
        movw    %sp,            %bp

        #   [bp]        num_remain_entries
        #   [bp + 2]    The offset of loading FAT12 entries
        #   [bp + 4]    Segment Base Address Loading FAT12 Entries
        #   [bp + 6]    Number of sectors to be searched
        #   [bp + 8]    The starting sector number to be searched (also current_section)
        #   [bp + 10]   Origin BP
        #   [bp + 12]   IP

        xorl    %eax,           %eax
        movw    %ds,            %ax
        shl     $4,             %eax
        movl    $global_buffer, %ebx
        subl    %eax,           %ebx
        movw    %bx,            2(%bp)

.copy_section_and_seek_file:
        cmpw    $0,             6(%bp)
        jz      .file_notfound
        decw    6(%bp)

        # seek for target file
        movw    8(%bp),         %ax
        pushw   %ax
        movw    $1,             %ax
        pushw   %ax
        movw    4(%bp),         %ax
        pushw   %ax
        movw    2(%bp),         %ax
        pushw   %ax
        call    copy_sectors 

        movw    4(%bp),         %ax
        movw    %ax,            %es
        movw    2(%bp),         %di

        movw    $16,            (%bp)   # NUM_ROOTDIR_ENTRIES_IN_SECT
.check_entry:
        cmpw    $0,             (%bp)
        jz      .next_sector
        decw    (%bp)

        movw    $11,            %cx
        call    cmp_str
        cmpw    $1,             %cx
        jz      .file_found
        jmp     .next_entry
.next_entry:
        addw    $32,            %di
        jmp     .check_entry
.next_sector:
        incw    8(%bp)
        jmp     .copy_section_and_seek_file

.file_notfound:
        movw    $0,             %ax
        jmp     .fin
.file_found:
        movw    4(%bp),         %ax
        movw    %ax,            %es
        addw    $0x1a,          %di
        movw    %es:(%di),      %ax
        jmp     .fin

.fin:
        movw    10(%bp),        %bx
        movw    %bx,            %bp
        addw    $12,            %sp
        ret

# --------------------------------
# input:
#   push (No.ax Sections)
#   push (numbers of sections to be read)
#   push es
#   push (memory address)
#
# int 13:
#   ah = 0, dl = driver no.     reset floppy
#
#   ah = 02h,       al = total sections being copying
#   ch = track no., cl = origin section no.
#   dh = head no.,  dl = driver no.(0 stand for A: driver)
#   es:bx = memory address
#---------------------------------
copy_sectors:
        pushw   %bp
        movw    %sp,            %bp
        
        pushw   %ax
        pushw   %bx
        pushw   %cx
        pushw   %dx
        pushw   %es

        movw    10(%bp),        %ax
        movb    $18,            %bl
        div     %bl
        incb    %ah
        movb    %ah,            %cl

        movb    %al,            %dh
        andb    $1,             %dh

        shrb    $1,             %al
        movb    %al,            %ch

        movb    $0,             %dl
        movw    4(%bp),         %bx
.start_copy:
        movw    6(%bp),         %ax
        movw    %ax,            %es
        movw    8(%bp),         %ax
        movb    $0x2,           %ah
        int     $0x13
        jc      .start_copy

        popw    %es
        popw    %dx
        popw    %cx
        popw    %bx
        popw    %ax
        popw    %bp
        ret     $8



# ----------------------------
# input:
#       ds:si   pointer1
#       es:di   pointer2
#       cx      length
# output:
#       cx(check) = 1(same)
#                 = 0(different)
# ----------------------------
cmp_str:
        pushw   %ax
        pushw   %si
        pushw   %di
        cld
1:
        cmpw    $0,             %cx
        jz      .cmp_succeed
        decw    %cx
        movb    %ds:(%si),      %al
        cmpb    %al,            %es:(%di)
        jz      2f
        jmp     .cmp_failed
2:
        incw    %di
        incw    %si
        jmp     1b
.cmp_succeed:
        movw    $1,             %cx
        jmp     .cmp_final
.cmp_failed:
        xor     %cx,            %cx
        jmp     .cmp_final
.cmp_final:
        popw    %di
        popw    %si
        popw    %ax
        ret


# ------------------------------------
# input:
#       ax          The starting sector number to be searched
#       es          Segment Base Address
#       di          The offset
# ------------------------------------
fat12_copy_to_mem:
        pushw   %di
        pushw   %bp             # used for (file_offset_data_sect)
        pushw   %bp             # used for (file_offset_all_sect)
        pushw   %bp
        movw    %sp,            %bp
        
        # now ax stores the first clus number of the target file in data sector
        movw    %ax,            2(%bp)
        addw    $19,            %ax
        addw    $14,            %ax
        subw    $2,             %ax     # No.0 and No.1 data sector are not used
        # now ax stores the sector number of the target file header
        movw    %ax,            (%bp)

.continue_copying:
        movw    (%bp),          %ax
        pushw   %ax
        movw    $1,             %ax
        pushw   %ax
        pushw   %es
        pushw   %di
        call    copy_sectors 
        
        movw    2(%bp),         %ax
        call    calculate_fat

        cmpw    $0xfff,         %ax
        jz      .finaish_loading
        movw    %ax,            2(%bp)
        addw    $19,            %ax
        addw    $14,            %ax
        subw    $2,             %ax     # data sector No.0 and No.1 are not used
        movw    %ax,            (%bp)
        addw    $512,           %di
        jmp     .continue_copying
.finaish_loading:
        addw    $4,             %sp
        popw    %bp
        popw    %di
        ret


# -----------------------------------
# input:
#       ax      (section no.)
# output:
#       ax      (FAT data)
# -----------------------------------
calculate_fat:
        pushl   %esi
        pushw   %bx
        pushw   %dx
        pushw   %es
        pushw   %bp
        pushw   %bp     # used for (cft_flag_is_odd)
        movw    %sp,            %bp

        xorw    %dx,            %dx
        movw    $3,             %bx
        mulw    %bx
        movw    $2,             %bx
        divw    %bx
        movw    %dx,            (%bp)

        # now ax stores the offset from FAT base address
        xorw    %dx,            %dx
        movw    $512,           %bx
        divw    %bx
        # now ax stores the order number of the sector
        # now dx stores the offset from the sector

        pushw   %dx

        movl    $global_buffer, %esi
        xorl    %ebx,           %ebx
        movw    %ds,            %bx
        shl     $4,             %ebx
        subl    %ebx,           %esi 
        
        addw    $1,             %ax
        pushw   %ax
        movw    $2,             %bx
        pushw   %bx
        pushw   %ds
        pushw   %si
        call    copy_sectors

        movw    %si,            %bx
        
        popw    %dx
        addw    %dx,            %bx
        movw    %ds:(%bx),      %ax
        cmpw    $1,             (%bp)
        jnz     .even
        shr     $4,             %ax
.even:
        andw    $0xfff,         %ax

        addw    $2,             %sp
        popw    %bp
        popw    %es
        popw    %dx
        popw    %bx
        popl    %esi
        ret
