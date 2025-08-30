@echo off

wcc -3 -ms -oailrt -s -oe=40 main.c
IF ERRORLEVEL 1 GOTO exit

REM wdis -s -a -l main.obj
REM IF ERRORLEVEL 1 GOTO exit

nasm -f obj -o timer.obj timer.asm -l timer.lst
nasm -f obj -o random.obj random.asm -l random.lst
nasm -f obj -o fx.obj fx.asm -l fx.lst
nasm -f obj -o keyb.obj keyb.asm -l keyb.lst

wlink name main.com system com op m option eliminate file { main.obj timer.obj random.obj fx.obj keyb.obj }

:exit
