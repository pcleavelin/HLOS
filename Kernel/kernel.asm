bits 16
org 0x8200

jmp start

%include "Kernel/IO/keyboard.inc"
%include "Kernel/main.inc"

start:
    cli
    mov ax, 0
    mov ds, ax
    mov es, ax ;set data segment and extra segment to cs (we can't move cs into ds and ex directly)

    call main

    push end_msg
    call _screen_print_string
    add sp, 2


forever:
    cli
    hlt
    jmp forever

reset_video_mode:
    push bp
    mov bp, sp

    xor ax, ax

    mov ah, 0 ;ah=0 (set video mode)
    mov al, 3 ;al=3 (80x25 mode)
    int 16 ;call the BIOS

    mov sp, bp
    pop bp
    ret

end_msg: db 'Rlease restart your computer.', 10, 13, 0
