;(C) Patrick Cleavelin <patrick@uptrainsoftware.com>
[bits 16]
[org 0x4000]

push bp
mov bp, sp

push message
call print_string

mov sp, bp
pop bp
ret

message: db 'This is david.asm!', 10, 13, 0

print_string:
    push bp
    mov bp, sp

	mov ax, word [bp+4]
	mov si, ax

	begin_loop:
		lodsb ;load next byte from si
		or al, al ;do a check on al to set flags
		jz end_loop ;if al is zero (null-terminator in string)
		mov ah, 14 ;ah=5 (teletype output)
		mov bx, 0 ;bx=0 (page number)
				  ;al (character) we get this from [si]
		int 16 ;print character to screen

		jmp begin_loop ;keep going
	end_loop:
		mov sp, bp ;return stack to pre-call state
		pop bp
		ret


