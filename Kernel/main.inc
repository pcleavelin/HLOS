%include "Kernel/Shell/shell.inc"

main:
    push bp
    mov bp, sp

    call reset_video_mode

    ;Do shell stuff
    call shell_do

    mov sp, bp
    pop bp
    ret

test_str: db 'This is the main part of the kernel', 10, 13, 0
