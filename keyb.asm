    bits 16
    cpu 386

segment _TEXT class=CODE

    global keyb_init_
keyb_init_:
    push ax
    push es
    push ds

    mov ax, 3509h
    int 21h
    mov [prev_isr], bx
    mov [prev_isr + 2], es

    cli

    mov ax, 2509h
    mov dx, keyb_isr ; ds is already equal to cs
    int 21h

    sti

    pop ds
    pop es
    pop ax
    ret

    global keyb_cleanup_
keyb_cleanup_:
    push ax
    push dx
    push ds

    cli

    mov dx, [prev_isr]
    mov ds, [prev_isr + 2]
    mov ax, 2509h
    int 21h

    sti

    pop ds
    pop dx
    pop ax
    ret

    extern keyb_key_

keyb_isr:
    pushad
    push ds
    push es

    mov ax, cs
    mov ds, ax
    mov es, ax

    in al, 0x60

    call keyb_key_

    mov al, 0x20
    out 0x20, al

    pop es
    pop ds
    popad
    iret

    align 4
prev_isr dd 0
