@echo off

wcc -3 -ms -oailrt -s -oe=64 main.c
IF ERRORLEVEL 1 GOTO exit

wdis -s -a -l main.obj
IF ERRORLEVEL 1 GOTO exit

nasm -f obj -o timer.obj timer.asm -l timer.lst
nasm -f obj -o random.obj random.asm -l random.lst

wlink name main.com system com option eliminate file { main.obj timer.obj random.obj }

:exit
