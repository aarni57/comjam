    bits 16
    cpu 386

segment _TEXT class=CODE

    global timer_init_
timer_init_:
    push bx
    push dx
    push es

    mov [update_func], ax

    mov ax, 351ch
    int 21h
    mov [prev_isr], bx
    mov [prev_isr + 2], es

    cli

    mov ax, 251ch
    mov dx, timer_isr ; ds is already equal to cs
    int 21h

    ; Set to 1165.215 Hz
    mov dx, 0x43
    mov al, 0x36
    out dx, al
    mov dx, 0x40
    mov al, 0x00 ; Divisor LSB
    out dx, al
    mov al, 0x04 ; MSB
    out dx, al

    sti

    pop es
    pop dx
    pop bx
    ret

    global timer_cleanup_
timer_cleanup_:
    push dx
    push ds

    cli

    mov dx, [prev_isr]
    mov ds, [prev_isr + 2]
    mov ax, 251ch
    int 21h

    mov dx, 0x43
    mov al, 0x36
    out dx, al
    mov dx, 0x40
    mov al, 0
    out dx, al
    out dx, al

    sti

    pop ds
    pop dx
    ret

    extern _timer_ticks

timer_isr:
    push eax
    inc dword [cs:_timer_ticks]
    mov eax, [cs:_timer_ticks]
    and eax, 0x3f
    jnz skip_update
    call [cs:update_func]
    skip_update:
    pop eax
    iret

    align 2
update_func dw 0

    align 4
prev_isr dd 0
