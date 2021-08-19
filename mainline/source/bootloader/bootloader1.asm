        org 7c00h
        jmp start
        nop

BS_OEMName              db      "LAB403JY"
BPB_BytesPerSec         dw      512
BPB_SecdPerClus         db      1
BPB_RsvdSecCnt          dw      1
BPB_NumFATs             db      2
BPB_RootEntCnt          dw      224
BPB_TotSec16            dw      2880
BPB_Media               db      0f0h
BPB_FATSz16             dw      9
BPB_SecsPerTrk          dw      18
BPB_NumHeads            dw      2
BPB_HiddSecs            dd      0
BPB_TotSec32            dd      0
BS_DrvNum               db      0
BS_Reserved             db      0
BS_BootSig              db      29h
BS_VolID                dd      0
BS_VolLab               db      "LAB403JY-0.1"
BS_FileSysType          db      "FAT12   "

; -------------------------------------------------------
FILE_LOAD_BASE          equ     9000h
FILE_LOAD_OFFSET        equ     0
FAT_LOAD_BASE           equ     8f00h
FAT_LOAD_OFFSET         equ     0

ROOTDIR_BEGIN_SECT      equ     19
NUM_ROOTDIR_SECT        equ     14
FAT1_BEGIN_SECT         equ     1

file_name:              db      "BL2     BIN", 0
current_section:        dw      0
num_remain_sections:    dw      0
num_remain_entries:     dw      0
file_offset_data_sect:  dw      0
file_offset_all_sect:   dw      0
; check FAT task
cft_flag_is_odd:        db      0
; strings
disp_booting:           db      "Booting ...", 0


start:
        mov     ax,             cs
        mov     ds,             ax
        mov     es,             ax
        mov     ss,             ax
        mov     sp,             7c00h
        
        ; reset floppy
        xor     ax,             ax
        xor     dx,             dx
        int     13h
        
        
        ; mov ax, 0
        ; mov si, disp_booting
        ; call disp_str
        
        ; check ROOTDIR_SECTNUM sections
        mov     word [num_remain_sections], NUM_ROOTDIR_SECT
        mov     word [current_section], ROOTDIR_BEGIN_SECT

copy_section_and_seek_file:
        cmp     word [num_remain_sections], 0
        jz      file_nofound
        dec     word [num_remain_sections]
        
        ; seek for loader.bin
        mov     ax,             word [current_section]
        push    ax
        mov     ax,             1
        push    ax
        mov     ax,             FILE_LOAD_BASE
        push    ax
        mov     ax,             FILE_LOAD_OFFSET
        push    ax
        call    copy_sections
        
        
        mov     ax,             FILE_LOAD_BASE
        mov     es,             ax
        mov     si,             file_name
        mov     di,             FILE_LOAD_OFFSET
        
        mov     ax,             word [BPB_BytesPerSec]
        mov     bx,             32
        div     bx
        mov     word [num_remain_entries], ax ; entry number in each section

check_entry:
        cmp     word [num_remain_entries], 0
        jz      next_section
        dec     word [num_remain_entries]
        
        mov     cx,             11
        call    string_cmp
        cmp     cx,             1
        jz      file_found
        jmp     next_entry

next_entry:
        add     di,             32
        mov     si,             file_name
        jmp     check_entry

next_section:
        inc     word [current_section]
        jmp     copy_section_and_seek_file


file_nofound:
        mov     ax,             0b800h
        mov     gs,             ax
        mov     ah,             0ch
        mov     al,             '!'
        mov     word [gs:((80 * 20 + 20) * 2)], ax
        jmp     $
file_found:
        mov     ax,             FILE_LOAD_BASE
        mov     es,             ax
        add     di,             1ah
        mov     cx,             word [es:di]
        ; now cx represents the first clus number of loader.bin in data sections
        mov     word [file_offset_data_sect], cx


        add     cx,             ROOTDIR_BEGIN_SECT
        add     cx,             NUM_ROOTDIR_SECT
        sub     cx,             2   ; Data section No.0 and No.1 not used
        ; now cx represents the section number of loader.bin header
        mov     word [file_offset_all_sect], cx

        mov     ax,             FILE_LOAD_BASE
        mov     es,             ax
        mov     bx,             FILE_LOAD_OFFSET

.continue_copying:
        mov     ax,             word [file_offset_all_sect]
        push    ax
        mov     ax,             1
        push    ax
        push    es
        push    bx
        call    copy_sections

        mov     ax,             word [file_offset_data_sect]
        call    calculate_FAT

        cmp     ax,             0fffh
        jz      .finish_loading
        mov     word [file_offset_data_sect], ax

        add     ax,             ROOTDIR_BEGIN_SECT
        add     ax,             NUM_ROOTDIR_SECT
        sub     ax,             2
        mov     word [file_offset_all_sect], ax
        add     bx,             word [BPB_BytesPerSec]
        jmp     .continue_copying

.finish_loading:
        mov     ax,             FILE_LOAD_BASE
        mov     es,             ax
        mov     bx,             FILE_LOAD_OFFSET

        jmp     FILE_LOAD_BASE:FILE_LOAD_OFFSET

; ----------------------------
; input:
;       ds:si   pointer1
;       es:di   pointer2
;       cx      length
; output:
;       cx(check) = 1(same)
;                 = 0(different)
; ----------------------------
string_cmp:
        push    ax
        push    si
        push    di
        cld
.1:
        cmp     cx,             0
        jz      .succeed
        dec     cx
        lodsb
        cmp     al,             byte [es:di]
        jz      .2
        jmp     .failed
.2:
        inc     di
        jmp     .1
.succeed:
        mov     cx,             1
        jmp     .final
.failed:
        xor     cx,             cx
        jmp     .final
.final:
        pop     di
        pop     si
        pop     ax
        ret

; -----------------------------------
; input:
;       ax(section no.)
; output:
;       ax(FAT data)
; -----------------------------------
calculate_FAT:
        push    bx
        push    dx
        push    es


        mov     bx,             3
        mul     bx
        mov     bx,             2
        div     bx
        mov     byte [cft_flag_is_odd], dl
        ; now ax represents the offset from FAT base address
        xor     dx,             dx
        mov     bx,             word [BPB_BytesPerSec]
        div     bx
        ; now ax represents the order number of the section
        ; now dx represents the offset from the section

        push    dx

        add     ax,             FAT1_BEGIN_SECT
        push    ax
        mov     bx,             2
        push    bx
        mov     bx,             FAT_LOAD_BASE
        mov     es,             bx
        push    es
        mov     bx,             FAT_LOAD_OFFSET
        push    bx
        call    copy_sections


        mov     ax,             FAT_LOAD_BASE
        mov     es,             ax
        mov     bx,             FAT_LOAD_OFFSET

        pop     dx
        add     bx,             dx
        mov     ax,             word [es:bx]
        cmp     byte [cft_flag_is_odd], 1
        jnz     .EVEN
        shr     ax,             4
.EVEN:
        and     ax,             0fffh

        pop     es
        pop     dx
        pop     bx
        ret


; --------------------------------
; input:
;   push (No.ax Sections)
;   push (numbers of sections to be read)
;   push es
;   push (memory address)
;
; int 13:
;   ah = 0, dl = driver no.     reset floppy
;
;   ah = 02h,       al = total sections being copying
;   ch = track no., cl = origin section no.
;   dh = head no.,  dl = driver no.(0 stand for A: driver)
;   es:bx = memory address
;---------------------------------
copy_sections:
        push    bp
        mov     bp,             sp

        push    ax
        push    bx
        push    cx
        push    dx
        push    es


        mov     ax,             word [bp + 10]
        mov     bl,             byte [BPB_SecsPerTrk]
        mov     bl,             18
        div     bl
        inc     ah
        mov     cl,             ah

        mov     dh,             al
        and     dh,             1

        shr     al,             1
        mov     ch,             al

        mov     dl,             byte [BS_DrvNum]
        mov     bx,             word [bp + 4]
.start_copy:
        mov     ax,             word [bp + 6]
        mov     es,             ax
        mov     ax,             word [bp + 8]
        mov     ah,             02h
        int     13h
        jc      .start_copy

        pop     es
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        pop     bp
        ret     8


times (510 - ($-$$)) db 0
Finish_Word     dw      0xaa55
