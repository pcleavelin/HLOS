;(C) Patrick Cleavelin <patrick@uptrainsoftware.com>
;This is a basic shell for the OS

global _keyboard_get_key
_keyboard_get_key:
    push bp
    mov bp, sp

    mov ah, 0
    int 0x16

    mov sp, bp
    pop bp
    ret
