    bits 16
    cpu 386

segment _TEXT class=CODE

    global timer_init_
timer_init_:
    push es
    push ds

    mov ax, 3508h
    int 21h
    mov [prev_isr], bx
    mov [prev_isr + 2], es

    cli

    mov ax, 2508h
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

    pop ds
    pop es
    ret

    global timer_cleanup_
timer_cleanup_:
    push es
    push ds

    cli

    mov dx, [prev_isr]
    mov ds, [prev_isr + 2]
    mov ax, 2508h
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
    pop es
    ret

    extern _timer_ticks
    extern timer_update_

timer_isr2:
    inc dword [cs:_timer_ticks]
    push ax
    mov al, 0x20
    out 0x20, al
    pop ax
    iret

timer_isr:
    push eax
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push sp
    push bp
    push ds
    push es
    push fs
    push gs
    push ss

    inc dword [cs:_timer_ticks]

    mov eax, [cs:_timer_ticks]
    and eax, 0x1f
    jnz skip_update
    call timer_update_
skip_update:

    mov al, 0x20
    out 0x20, al

    pop ss
    pop gs
    pop fs
    pop es
    pop ds
    pop bp
    pop sp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax
    iret

    align 4
prev_isr dd 0
