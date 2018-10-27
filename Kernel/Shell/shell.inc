;(C) Patrick Cleavelin <patrick@uptrainsoftware.com>
;This is a basic shell for the OS

%include "Kernel/IO/fat8.inc"

%define shell_input_buffer_length 16

shell_do:
    push bp
    mov bp, sp
    sub sp, 2

    .shell_loop:
        call shell_print_text_entry
        call shell_clear_input_buffer

        mov word[bp-2], 0
        .shell_key_loop:
            call _keyboard_get_key

            cmp al, 0x08
            je .shell_key_loop_backspace

            jmp .shell_key_add_to_buffer

            .shell_key_loop_backspace:
                cmp word[bp-2], 0
                je .shell_key_loop

                mov bx, word[bp-2]
                mov byte[shell_input_buffer+bx], 0

                dec word[bp-2]
                push word 0x01
                call _screen_move_cursor_left
                add sp, 2

                push word 0x8
                push word 0
                call _screen_write_character_with_color
                add sp, 4

                push word 0x01
                call _screen_move_cursor_left
                add sp, 2

                jmp .shell_key_loop

            .shell_key_add_to_buffer:
                ; Check if we have reached the size of the input buffer
                cmp word[bp-2], 7
                je .shell_key_loop_done

                ; add typed key to input buffer
                mov bx, word[bp-2]
                mov [shell_input_buffer+bx], al
                cmp al, 13

                je .shell_key_loop_done

                mov ax, shell_input_buffer
                add ax, word[bp-2]
                
                inc word[bp-2]

                push ax 
                call _screen_print_string
                add sp, 2

            
            jmp .shell_key_loop
        .shell_key_loop_done:

        call _screen_cursor_newline
        call _screen_cursor_carrage_return
        call shell_handle_input
        jmp .shell_loop

    mov sp, bp
    pop bp
    ret

shell_print_text_entry:
    push bp
    mov bp, sp

    push shell_text_entry_str
    call _screen_print_string
    add sp, 2

    mov sp, bp
    pop bp
    ret

shell_clear_input_buffer:
    push bp
    mov bp, sp

    mov bx, shell_input_buffer_length-1
    .clear_loop:
        mov byte[shell_input_buffer+bx], 0
        dec bx
        jne .clear_loop

    mov sp, bp
    pop bp
    ret

shell_handle_input:
    push bp
    mov bp, sp

    push shell_input_buffer
    call fat8_read_file
    add sp, 2

    cmp ax, 1 
    jle .skip_call
    call 0x4000
    .skip_call:

    mov sp, bp
    pop bp
    ret

shell_text_entry_str: db 10, 13, '(simple shell)-> ', 0
shell_input_buffer: times shell_input_buffer_length db 0
