@echo off

wcc -3 -ms -oailrt -s -oe=40 -fp3 -wx main.c
IF ERRORLEVEL 1 GOTO exit

wdis -s -a -l main.obj
IF ERRORLEVEL 1 GOTO exit

nasm -f obj -o timer.obj timer.asm

wlink name main.com system com option eliminate file { main.obj timer.obj }

:exit
