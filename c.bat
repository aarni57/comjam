@echo off

wcc -3 -ms -oirlx main.c
IF ERRORLEVEL 1 GOTO exit

wdis -s -a -l main.obj
IF ERRORLEVEL 1 GOTO exit

wlink system com option eliminate, vfremoval file main.obj

:exit
