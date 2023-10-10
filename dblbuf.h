static uint8_t __far* dblbuf = NULL;

static inline int dblbuf_allocate() {
    dblbuf = (uint8_t __far*)_fmalloc(SCREEN_NUM_PIXELS);
    return dblbuf != NULL;
}

static inline void dblbuf_release() {
    _ffree(dblbuf);
    dblbuf = NULL;
}

static inline void dblbuf_clear() {
#if !defined(INLINE_ASM)
    _fmemset(dblbuf, 0x08, SCREEN_NUM_PIXELS);
#else
    __asm {
        push es
        push edi

        mov eax, 0x08080808

        mov edx, dblbuf
        movzx edi, dx
        shr edx, 16
        mov es, dx

        mov cx, SCREEN_NUM_PIXELS
        shr cx, 2

        rep stosd

        pop edi
        pop es
    }
#endif
}

static inline void dblbuf_copy_to_screen() {
#if !defined(INLINE_ASM)
    _fmemcpy(VGA, dblbuf, SCREEN_NUM_PIXELS);
#else
    __asm {
        push es
        push ds
        push edi
        push esi

        mov edx, dblbuf
        movzx esi, dx
        shr edx, 16
        mov ds, dx

        mov dx, 0xa000
        mov es, dx
        xor edi, edi

        mov cx, SCREEN_NUM_PIXELS
        shr cx, 2

        rep movsd

        pop esi
        pop edi
        pop ds
        pop es
    }
#endif
}
