    bits 16
    cpu 386

segment _TEXT class=CODE

%define initial_value 0xcafebabe

    align 4
xorshift32_state dd initial_value

%macro update 0
    mov eax, [xorshift32_state]
    mov edx, eax
    shl edx, 13
    xor eax, edx
    mov edx, eax
    shr edx, 17
    xor eax, edx
    mov edx, eax
    shl edx, 5
    xor eax, edx
    mov [xorshift32_state], eax
%endmacro

%macro result_to_dx_ax 0
    mov edx, eax
    shr edx, 16
%endmacro

    global xorshift32_
xorshift32_:
    update
    result_to_dx_ax
    ret

    global fx_random_one_
fx_random_one_:
    update
    xor dx, dx
    ret

%macro fx_signed_one 0
    and eax, 0x1ffff
    sub eax, 0x10000
%endmacro

    global fx_random_signed_one_
fx_random_signed_one_:
    update
    fx_signed_one
    result_to_dx_ax
    ret

    global random_debris_x_
random_debris_x_:
    update
    fx_signed_one
    sar eax, 5
    result_to_dx_ax
    ret

    global xorshift32_reset_
xorshift32_reset_:
    mov dword [xorshift32_state], initial_value
    ret
