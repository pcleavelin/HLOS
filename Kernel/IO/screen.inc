;(C) Patrick Cleavelin <patrick@uptrainsoftware.com>
;Screen functionality

;unsigned word _screen_get_cursor_pos(void);
global _screen_get_cursor_pos
_screen_get_cursor_pos:
    push bp
    mov bp, sp

    mov ah, 3
    mov bh, 0
    int 0x10

    mov ax, dx

    mov sp, bp
    pop bp
    ret

;void _screen_set_cursor_pos(byte column, byte row);
global _screen_set_cursor_pos
_screen_set_cursor_pos:
    push bp
    mov bp, sp

    mov dh, byte[bp+5]
    mov dl, byte[bp+4]
    mov ah, 2
    mov bh, 0
    int 0x10

    mov sp, bp
    pop bp
    ret

;void _screen_cursor_newline()
global _screen_cursor_newline
_screen_cursor_newline:
    push bp
    mov bp, sp

    push word 1
    call _screen_move_cursor_down
    add sp, 2

    mov sp, bp
    pop bp
    ret

;void _screen_cursor_carrage_return()
global _screen_cursor_carrage_return
_screen_cursor_carrage_return:
    push bp
    mov bp, sp

    call _screen_get_cursor_pos
    mov al, 0

    push ax
    call _screen_set_cursor_pos
    add sp, 2

    mov sp, bp
    pop bp
    ret

;void _screen_move_cursor_up(word amount)
global _screen_move_cursor_up
_screen_move_cursor_up:
    push bp
    mov bp, sp

    call _screen_get_cursor_pos
    sub ah, byte[bp+4]

    push ax
    call _screen_set_cursor_pos
    add sp, 2

    mov sp, bp
    pop bp
    ret

;void _screen_move_cursor_down(word amount)
global _screen_move_cursor_down
_screen_move_cursor_down:
    push bp
    mov bp, sp

    call _screen_get_cursor_pos
    add ah, byte[bp+4]

    push ax
    call _screen_set_cursor_pos
    add sp, 2

    mov sp, bp
    pop bp
    ret


;void _screen_move_cursor_left(word amount)
global _screen_move_cursor_left
_screen_move_cursor_left:
    push bp
    mov bp, sp

    call _screen_get_cursor_pos
    sub al, byte[bp+4]

    push ax
    call _screen_set_cursor_pos
    add sp, 2

    mov sp, bp
    pop bp
    ret

;void _screen_move_cursor_right(word amount)
global _screen_move_cursor_right
_screen_move_cursor_right:
    push bp
    mov bp, sp

    call _screen_get_cursor_pos
    add al, byte[bp+4]

    push ax
    call _screen_set_cursor_pos
    add sp, 2

    mov sp, bp
    pop bp
    ret

;void _screen_write_character_with_color(word c, word color);
global _screen_write_character_with_color
_screen_write_character_with_color:
    push bp
    mov bp, sp

    ; do we have a new line feed?
    mov al, byte[bp+4]
    cmp al, 10
    jne .screen_write_char_with_color_no_newline

    call _screen_cursor_newline
    jmp .screen_write_charcolor_done

    .screen_write_char_with_color_no_newline:

    mov al, byte[bp+4]
    cmp al, 13
    jne .screen_write_char_with_color_no_carrage_return

    call _screen_cursor_carrage_return
    jmp .screen_write_charcolor_done

    .screen_write_char_with_color_no_carrage_return:
        mov ah, 0x09
        mov al, byte[bp+4]
        mov bl, byte[bp+6]
        mov cx, 1
        mov bh, 0
        int 0x10

        push word 0x01
        call _screen_move_cursor_right
        add sp, 2

    .screen_write_charcolor_done:
    mov sp, bp
    pop bp
    ret

;void _screen_write_character(word c);
global _screen_write_character
_screen_write_character:
    push bp
    mov bp, sp

    ; do we have a new line feed?
    mov al, byte[bp+4]
    cmp al, 10
    jne .screen_write_char_no_newline

    call _screen_cursor_newline
    jmp .screen_write_char_done

    .screen_write_char_no_newline:

        mov al, byte[bp+4]
        cmp al, 13
        jne .screen_write_char_no_carrage_return

        call _screen_cursor_carrage_return
        jmp .screen_write_char_done

    .screen_write_char_no_carrage_return:
        mov ah, 0x0A
        mov al, byte[bp+4]
        mov cx, 1
        mov bh, 0
        int 0x10

        push word 0x01
        call _screen_move_cursor_right
        add sp, 2

    .screen_write_char_done:
    mov sp, bp
    pop bp
    ret

;void screen_print_string(char* string)
global _screen_print_string
_screen_print_string:
    push bp
    mov bp, sp
    push si

	mov ax, word [bp+4]
	mov si, ax

	.screen_print_string_begin_loop:
		lodsb ;load next byte from si
		or al, al ;do a check on al to set flags
		jz .screen_print_string_end_loop ;if al is zero (null-terminator in string)
		
        push ax
        call _screen_write_character
        add sp, 2

		jmp .screen_print_string_begin_loop ;keep going
	.screen_print_string_end_loop:
        pop si

		mov sp, bp ;return stack to pre-call state
		pop bp
		ret