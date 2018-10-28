;(C) Patrick Cleavelin <patrick@uptrainsoftware.com>

extern _main
global start

section .text
start:
    push bp
    mov bp, sp

    call _main

    mov sp, bp
    pop bp
    ret

%include "Kernel/IO/keyboard.inc"
%include "Kernel/IO/fat8.inc"