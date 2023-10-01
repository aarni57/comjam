    bits 16
    cpu 386

segment _TEXT class=CODE

    global fx_mul_
fx_mul_:
    xchg ax, dx
    sal eax, 16
    mov ax, dx
    sal ecx, 16
    mov cx, bx
    imul ecx
    shrd eax, edx, 16
    mov edx, eax
    shr edx, 16
    ret

    global fx_mul_ptr_
fx_mul_ptr_:
    push si
    mov si, ax
    mov eax, [si]
    mov si, dx
    mov edx, [si]
    imul edx
    shrd eax, edx, 16
    mov edx, eax
    shr edx, 16
    pop si
    ret

    global fx_pow2_
fx_pow2_:
    rol eax, 16
    mov ax, dx
    rol eax, 16
    imul eax
    shrd eax, edx, 16
    mov edx, eax
    shr edx, 16
    ret
