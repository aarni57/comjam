static inline void clear_memory(uint8_t __far* memory, uint8_t value, uint16_t num) {
#if !defined(INLINE_ASM)
    _fmemset(memory, value, num);
#else
    __asm {
        push es
        push edi

        mov al, value
        mov ah, value
        shl eax, 16
        mov al, value
        mov ah, value

        mov edx, memory
        movzx edi, dx
        shr edx, 16
        mov es, dx

        mov cx, num
        shr cx, 2

        rep stosd

        pop edi
        pop es
    }
#endif
}

static inline void copy_memory(uint8_t __far* tgt, const uint8_t __far* src, uint16_t num) {
#if !defined(INLINE_ASM)
    _fmemcpy(tgt, src, num);
#else
    __asm {
        push es
        push ds
        push edi
        push esi

        mov edx, src
        movzx esi, dx
        shr edx, 16
        mov ds, dx

        mov edx, tgt
        movzx edi, dx
        shr edx, 16
        mov es, dx

        mov cx, num
        shr cx, 2

        rep movsd

        pop esi
        pop edi
        pop ds
        pop es
    }
#endif
}
