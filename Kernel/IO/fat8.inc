;(C) Patrick Cleavelin <patrick@uptrainsoftware.com>
;This is the FAT8 driver

%include "Kernel/IO/screen.inc"

%define BYTES_PER_SECTOR 512
%define NUMBER_OF_FAT_ENTRIES 256
%define NUMBER_OF_ROOT_DIR_SECTORS 2

%define NUMBER_OF_FILE_SECTORS 252
%define NUMBER_OF_RESERVED_SECTORS 4

%define FAT_FREE_SECTOR 0x00
%define FAT_RESERVED_SECTOR 0x01

%define ROOT_FREE_ENTRY 0x0

%define LOAD_SEGMENT 0x0000
%define ROOT_LOAD_ADDR 0x3e00
%define FAT_LOAD_ADDR 0x3c00
%define FILE_LOAD_ADDR 0x4000

%define FAT_SECTOR_LOC 0x1
%define ROOT_SECTOR_LOC 0x2

;The DAP (for reading sectors from floppy)
dap: db 0x10
dap_unused: db 0
dap_num_to_read: dw 1
dap_read_to_offset: dw 0x0
dap_read_to_segment: dw 0x0
dap_sector_to_read_lo: dd 0x00000000
dap_sector_to_read_hi: dd 0x00000000

;unsigned char fat8_read_file(const char* filename, unsigned int load_addr);
global _fat8_read_file
_fat8_read_file:
    push bp
    mov bp, sp
    sub sp, 4

    mov word[bp-2], 0 
    mov word[bp-4], 0
    .fat8_root_read_loop:
        mov bx, word[bp-2]
        cmp bx, NUMBER_OF_ROOT_DIR_SECTORS-1
        je .fat8_root_read_no_filename

        ;bx is which root dir sector to read from (0 or 1)
        push bx
        call _fat8_load_root_dir
        add sp, 2
        
        ;Move the first parameter into ax then pass it to the
        ;find file function
        mov ax, word[bp+4]
        push ax
        call fat8_find_file
        add sp, 2
        mov word[bp-4], ax
        cmp ax, 0
        jne .fat8_root_read_loop_done

        ;the file wasn't in the first root dir sector
        inc word[bp-2]
        jmp .fat8_root_read_loop

        .fat8_root_read_no_filename:
            ;couldn't find file, return 1
            push fat8_could_not_file_file_msg
            call _screen_print_string
            add sp, 2

            mov ax, 0x0001 
            jmp .fat8_read_file_return

    .fat8_root_read_loop_done:
        push fat8_found_match_msg
        call _screen_print_string
        add sp, 2
        
        push word[bp+6]
        push word[bp-4]
        call fat8_load_file_from_disk
        add sp, 2

    .fat8_read_file_return:
    mov sp, bp
    pop bp
    ret

;word fat8_load_file_from_disk(word fat_index, word load_addr)
fat8_load_file_from_disk:
    push bp
    mov bp, sp

    call fat8_load_fat

    mov bx, 0
    mov si, FAT_LOAD_ADDR
    .fat8_load_file_loop_sectors:
        cmp bx, 0xff
        jne .fat8_load_file_not_failed

        push bx
        push fat8_load_file_failed_msg
        call _screen_print_string
        add sp, 2
        pop bx

        mov ax, 0x0
        jmp .fat8_load_file_done

        .fat8_load_file_not_failed:
        inc bx

        ;load fat entry into ax
        lodsb

        ;check if the is the fat entry we are looking for
        cmp ax, word[bp+4]
        jne .fat8_load_file_loop_sectors

        .fat8_load_file_read_sectors:
            ;make bx a pointer into the FAT
            dec bx
            add bx, FAT_LOAD_ADDR

            mov ax, word[bp+6]
            .fat8_load_file_loop_read:
                mov [dap_read_to_offset], ax

                ;save ax
                push ax

                ;load sector location into ax
                xor ax, ax
                mov al, byte[bx]

                ;check if sector is zero (that means EOF)
                cmp al, 0
                je .fat8_load_file_read_done

                mov [dap_sector_to_read_lo], al

                push bx
                call fat8_read_disk
                pop bx

                ;increment FAT pointer
                inc bx

                ;get where we are loading the file back
                pop ax

                ;move 512 bytes
                add ax, 0x200

                jmp .fat8_load_file_loop_read

            .fat8_load_file_read_done:
                ;fix the stack
                pop ax

                ;return the location of the file in memory
                mov ax, word[bp+6]

    .fat8_load_file_done:
    mov sp, bp
    pop bp
    ret

;void fat8_load_fat(int sector)
fat8_load_fat:
    push bp
    mov bp, sp

    mov dword[dap_sector_to_read_lo], FAT_SECTOR_LOC
    mov word[dap_read_to_segment], LOAD_SEGMENT
    mov word[dap_read_to_offset], FAT_LOAD_ADDR
    call fat8_read_disk

    mov sp, bp
    pop bp
    ret


;void fat8_load_root_dir(int sector)
global _fat8_load_root_dir
_fat8_load_root_dir:
    push bp
    mov bp, sp

    mov dword[dap_sector_to_read_lo], ROOT_SECTOR_LOC
    mov word[dap_read_to_segment], LOAD_SEGMENT
    mov word[dap_read_to_offset], ROOT_LOAD_ADDR
    call fat8_read_disk

    mov sp, bp
    pop bp
    ret

;unsigned char fat8_find_file(const char* filename)
fat8_find_file:
    push bp
    mov bp, sp
    sub sp, 2
 
    ;cx will store the fat index/sector location of file
    mov cx, 0

    ;stores the current character
    mov byte[bp-1], 0

    ;stores the index into 'filename'
    mov byte[bp-2], 0

    mov si, ROOT_LOAD_ADDR
    .fat8_read_dir:
        .fat8_read_dir_entry_loop:
            ;Get character in filename
            xor ax, ax
            mov bx, word[bp+4]
            
            mov al, byte[bp-2]
            add bx, ax
            
            mov al, byte[bx]
            mov byte[bp-1], al

            ;Get character from root directory 
            xor ax, ax
            lodsb

            ;Compare character from root dir entry with filename
            cmp al, byte[bp-1]
            jne .fat8_read_dir_entry_no_match

            ;if the character was a match
            inc cx
            inc byte[bp-2]
            cmp byte[bp-2], 7
            je .fat8_read_dir_entry_match
            jmp .fat8_read_dir_entry_loop
            .fat8_read_dir_entry_no_match:

                push cx
                push fat8_not_match_msg
                call _screen_print_string
                add sp, 2
                pop cx

                ;Go to the next root entry (each is 8 bytes)
                add cx, 8

                ;Make sure we line up with the first byte by subtracting
                ;the index into the filename
                xor ax, ax
                mov al, byte[bp-2]
                sub cx, ax
                mov byte[bp-2], 0

                ;Now update si
                mov si, ROOT_LOAD_ADDR
                add si, cx

                ;Stop after 256 entries
                cmp cx, 0xff
                jge .fat8_read_dir_entry_reached_end
                jmp .fat8_read_dir_entry_loop

            .fat8_read_dir_entry_match:
                push cx

                push fat8_found_match_msg
                call _screen_print_string
                add sp, 2

                pop cx

                ;Move the sector number in to ax and return
                mov si, ROOT_LOAD_ADDR
                add si, cx
                lodsb
                and ax, 0x00ff
                jmp .fat8_read_dir_done
                
            .fat8_read_dir_entry_reached_end:
               push fat8_reached_end_msg
               call _screen_print_string
               add sp, 2
               mov ax, 0x0000
                

    .fat8_read_dir_done:
	mov sp, bp
    pop bp
	ret 

fat8_read_disk:
    push bp
    mov bp, sp

	call fat8_reset_disk
	mov si, dap
	mov ah, 0x42
	mov dl, 0x80
	int 0x13

    mov sp, bp
    pop bp
	ret

fat8_reset_disk:
    push bp
    mov bp, sp

	mov ax, 0x0 ;Reset disk drive
	int 0x13

    mov sp, bp
    pop bp
	ret

fat8_could_not_file_file_msg: db "Couldn't find file", 10, 13, 0
fat8_reached_end_msg: db 10, 13, "Reached end of root dir", 0xA, 0xD, 0
fat8_not_match_msg: db "Not a match...", 13, 0
fat8_found_match_msg: db "Found match", 13, 0
fat8_load_file_failed_msg: db "Failed to load file", 10, 13, 0
