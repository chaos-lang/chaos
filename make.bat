@ECHO OFF
IF [%1]==[] (
    SET compiler=gcc
    SET extra_flags=-D__USE_MINGW_ANSI_STDIO
) ELSE IF [%1]==[dev] (
    SET compiler=gcc
    SET extra_flags=-D__USE_MINGW_ANSI_STDIO -ggdb
) ELSE IF [%1]==[clang] (
    SET compiler=clang-cl
) ELSE IF [%1]==[clang-dev] (
    SET compiler=clang-cl
    SET extra_flags=-ggdb
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
) ELSE IF [%1]==[test-extensions-windows-gcc] (
    CD tests\extensions\spells
    gcc -shared -fPIC example.c -o example.o
    gcc -c example.c
    gcc -shared -o example.dll example.o -Wl,--out-implib,libexample.a
    CD ..\..\..
    chaos tests\extensions\test.kaos
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
%compiler% -Iloops -Ifunctions -Imodules -o chaos.exe chaos.tab.c lex.yy.c loops/*.c functions/*.c modules/*.c utilities/*.c symbol.c errors.c Chaos.c %extra_flags%
IF errorlevel 1 (
    EXIT /B 1
)

EXIT /B 0
