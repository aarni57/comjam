## itch.io DOS COM Jam - September 2023

Space simulation game prototype, heavily inspired by Wing Commander: Privateer

Compiles to a COM executable (48 542 bytes) using Open Watcom 1.9
Needs a 386 in order to start, but a fast 486 or Pentium is recommended. Doesn't use FPU.

A lot of the code is written in inline assembly as the 16-bit compiler doesn't produce assembly from C that would use 32-bit registers or instructions. There is however C-versions included along the inline assembly. Preprocessor definition 'INLINE_ASM' can be removed in main.c to use those C-versions.

[YouTube video](https://youtu.be/5uebtEZ8kFc)

[aarnig.itch.io/dos-com-jam](https://aarnig.itch.io/dos-com-jam)
