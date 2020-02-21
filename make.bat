@ECHO OFF
IF [%1]==[] (
    SET compiler=gcc
) ELSE IF [%1]==[clang] (
    SET compiler=clang
) ELSE IF [%1]==[requirements] (
    choco install winflexbison3 --force --confirm
    choco install mingw --force --confirm
    choco install llvm --force --confirm
    EXIT /B 0
) ELSE IF [%1]==[install] (
    move chaos.exe C:\Windows\System32\
    EXIT /B 0
)
win_flex chaos.l
win_bison -d chaos.y
%compiler% -Werror -Iloops -Ifunctions -o chaos chaos.tab.c lex.yy.c loops/*.c functions/*.c utilities/*.c symbol.c errors.c