;(C) Patrick Cleavelin <patrick@uptrainsoftware.com>

[bits 16]

jmp start

times 3 - ($-$$) db 0
%include "Kernel/Bootloader/BPB.inc"

start:
    mov ax, 0x7c0
	mov ds, ax
	mov es, ax ;set data segment and extra segment to cs (we can't move cs into
			   ;ds and ex directly)

	mov ah, 0x0 ;ah=0 (set video mode)
	mov al, 0x3 ;al=3 (80x25 mode)
	int 0x10 ;call the BIOS

	mov si, hello_msg
	call print_string

	mov ah, 0x41
	mov bx, 0x55aa
	mov dl, 0x80
	int 0x13
	jnc .supported

	mov si, bios_not_supported
	call print_string
	jmp forever

	.supported:
		;Read first sector of root directory into RAM at 0x200
		mov al, [root_sector]
		mov [dap_sector_to_read_lo], al
		call read_disk

		;read the FAT into ram at 0x300
		mov al, [fat_sector]
		mov [dap_sector_to_read_lo], al
		mov ax, 0x300
		mov [dap_read_to_offset], ax
		call read_disk

	.read_dir:
		;Check for our KERNELB
		mov bx, 0

		;Load pointer to root directory in si
		;so that, we can check the file names
		mov si, [root_loc]
		.loop:
		    ; Get character of file name
			lodsb

			cmp al, [kernel_filename+bx]
			jne .not_same_character

			.same_character:
				inc bx
				cmp bx, 7
				je .found_file
				jmp .loop

			.not_same_character:
				mov ax, [root_loc]
				add ax, 8
				mov [root_loc], ax
				jmp .read_dir


		.found_file:
			mov si, found_file_msg
			call print_string

			;Begin loading each sector into memory
			mov bx, [root_loc]

			;The 8th byte is the sector location
			add bx, 7
			mov al, [bx]
			mov [cluster], al

			;Now find the same sector location in the FAT
			mov si, [fat_loc]
			mov bx, 0
			xor ax, ax
			.loop_sectors:
			    ;If we have gone through all FAT entries and still haven't found
			    ;the sector location, throw a fit of rage
			    cmp bx, 255
			    je fatal_error

                inc bx

			    ;Load an entry into ax
                lodsb

                ;Compare the sector in the root directory with the one in the FAT
                cmp ax, word [cluster]
                jnz .loop_sectors

            mov si, loading_kernel_msg
            call print_string

            .read_sectors:
                ;Make bx the pointer into the FAT
                dec bx
                add bx, [fat_loc]

                mov ax, 0x600
                .loop_read:
                    mov [dap_read_to_offset], ax

                    ;Save ax so we can load the sector location into it
                    push ax

                    ;Load sector location into ax
                    xor ax, ax
                    mov al, byte [bx]

                    mov [dap_sector_to_read_lo], byte al
                    push bx
                    call read_disk
                    pop bx

                    mov si, loaded_sector_msg
                    call print_string

                    ;Get next sector and check if it's zero (for now that means the end of the file)
                    inc bx
                    mov ax, [bx]
                    cmp ax, 0
                    jz .read_done

                    ;Get ax back
                    pop ax
                    add ax, 0x200

                    jmp .loop_read

                .read_done:
                    pop ax
                    jmp 0x0000:0x8200

fatal_error:
    mov si, fatal_error_msg
    call print_string

forever:
	jmp forever

read_disk:
	push ax
	push dx
	call reset_disk
	mov si, dap
	mov ah, 0x42
	mov dl, 0x80
	int 0x13
	pop dx
	pop ax
	ret

reset_disk:
	push ax
	mov ax, 0x0 ;Reset disk drive
	int 0x13
	pop ax
	ret


print_string:
	pusha ;preserve registers

	mov bp, sp ;setup stack
	.loop:
		lodsb ;load next byte from si
		or al, al ;do a check on al to set flags
		jz .end_loop ;if al is zero (null-terminator in string)
		mov ah, 0x0e ;ah=5 (teletype output)
		mov bx, 0 ;bx=0 (page number)
				  ;al (character) we get this from [si]
		int 0x10 ;print character to screen
		jmp .loop ;keep going
	.end_loop:
		mov sp, bp ;return stack to pre-call state
		popa
		ret


;The DAP (for reading sectors from floppy)
dap: db 0x10
dap_unused: db 0
dap_num_to_read: dw 1
dap_read_to_offset: dw 0x200
dap_read_to_segment: dw 0x7c0
dap_sector_to_read_lo: dd 0x00000000
dap_sector_to_read_hi: dd 0x00000000

fat_loc: dw 0x300 ;Location to load each sector of the FAT into
fat_sector: db 1
root_loc: dw 0x200
root_sector: db 2   ;The root directory starts at the 3rd sector

cluster: dw 0

found_file_msg: db "Found Kernel...", 10, 13, 0
loading_kernel_msg: db "Loading Kernel into memory...", 10, 13, 0
loaded_sector_msg: db "Loaded Sector...", 0
kernel_filename: db "KERNELB"
hello_msg: db "Bootloader v0.1", 10, 13, 0
bios_not_supported: db "LBA Extensions not supported", 10, 13, 0
fatal_error_msg: db "Fatal Error", 10, 13, 0
times 510 - ($-$$) db 0 ;nasm macro that fills the remaining sector space with
					   ;zeros
dw 0xaa55 ;boot signature BIOS looks for when searching for boot device
