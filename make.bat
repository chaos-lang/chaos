@ECHO OFF
IF [%1]==[] (
    SET compiler=gcc
) ELSE IF [%1]==[clang] (
    SET compiler=clang-cl
) ELSE IF [%1]==[requirements] (
    choco install winflexbison3 --confirm
    choco install mingw --confirm
    choco install llvm --confirm
    EXIT /B 0
) ELSE IF [%1]==[install] (
    move chaos.exe C:\Windows\System32\
    EXIT /B 0
) ELSE IF [%1]==[test] (
    CALL tests\run.bat
    EXIT /B 0
) ELSE IF [%1]==[memcheck] (
    CD tests
    CALL memcheck.bat
    CD ..
    EXIT /B 0
) ELSE IF [%1]==[clean] (
    DEL chaos.exe chaos.tab.c lex.yy.c chaos.tab.h
    EXIT /B 0
)
win_flex --wincompat chaos.l
win_bison -d chaos.y
%compiler% -Iloops -Ifunctions -o chaos.exe chaos.tab.c lex.yy.c loops/*.c functions/*.c utilities/*.c symbol.c errors.c
IF errorlevel 1 (
    EXIT /B 1
)
