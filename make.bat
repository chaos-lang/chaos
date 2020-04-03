IF [%1]==[] (
    SET compiler=gcc
) ELSE IF [%1]==[clang] (
    SET compiler=clang-cl
) ELSE IF [%1]==[requirements] (
    choco install winflexbison3 --confirm
    IF errorlevel 1 (
        EXIT /B 1
    )
    choco install mingw --confirm
    IF errorlevel 1 (
        EXIT /B 1
    )
    choco install llvm --confirm
    IF errorlevel 1 (
        EXIT /B 1
    )

    EXIT /B 0
) ELSE IF [%1]==[install] (
    move chaos.exe C:\Windows\System32\
    IF errorlevel 1 (
        EXIT /B 1
    ) ELSE (
        EXIT /B 0
    )
) ELSE IF [%1]==[test] (
    CALL tests\run.bat
    IF errorlevel 1 (
        EXIT /B 1
    ) ELSE (
        EXIT /B 0
    )
) ELSE IF [%1]==[memcheck] (
    CD tests
    CALL memcheck.bat
    IF errorlevel 1 (
        CD ..
        EXIT /B 1
    ) ELSE (
        CD ..
        EXIT /B 0
    )
) ELSE IF [%1]==[clean] (
    DEL chaos.exe chaos.tab.c lex.yy.c chaos.tab.h
    EXIT /B 0
)

win_flex --wincompat chaos.l
IF errorlevel 1 (
    EXIT /B 1
)
win_bison -d chaos.y
IF errorlevel 1 (
    EXIT /B 1
)
%compiler% -Iloops -Ifunctions -Imodules -o chaos.exe chaos.tab.c lex.yy.c loops/*.c functions/*.c modules/*.c utilities/*.c symbol.c errors.c
IF errorlevel 1 (
    EXIT /B 1
)

EXIT /B 0
