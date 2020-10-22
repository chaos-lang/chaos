@ECHO OFF
setlocal EnableDelayedExpansion

IF [%1]==[] (
    SET compiler=gcc
    SET extra_flags=-D__USE_MINGW_ANSI_STDIO
) ELSE IF [%1]==[dev] (
    SET compiler=gcc
    SET extra_flags=-D__USE_MINGW_ANSI_STDIO -ggdb
) ELSE IF [%1]==[clang] (
    SET compiler=clang
) ELSE IF [%1]==[clang-dev] (
    SET compiler=clang
    SET extra_flags=-ggdb
) ELSE IF [%1]==[requirements] (
    CALL :InstallRequirements
    IF errorlevel 1 (
        EXIT /B 1
    )
    EXIT /B 0
) ELSE IF [%1]==[requirements-dev] (
    CALL :InstallRequirementsDev
    IF errorlevel 1 (
        EXIT /B 1
    )
    EXIT /B 0
) ELSE IF [%1]==[install] (
    CALL :InstallRequirementsDev
    IF errorlevel 1 (
        EXIT /B 1
    )
    MOVE chaos.exe %windir%\System32\
    IF errorlevel 1 (
        EXIT /B 1
    ) ELSE (
        EXIT /B 0
    )
) ELSE IF [%1]==[uninstall] (
    CALL :Uninstall
    IF errorlevel 1 (
        EXIT /B 1
    ) ELSE (
        EXIT /B 0
    )
) ELSE IF [%1]==[test] (
    CALL tests\interpreter.bat
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
    DEL chaos.exe parser.tab.c parser.tab.h lex.yy.c
    EXIT /B 0
) ELSE IF [%1]==[test-extensions-windows-gcc] (
    CD tests\extensions\spells\example
    gcc -shared -fPIC example.c -o example.o
    gcc -c example.c
    gcc -shared -o example.dll example.o -Wl,--out-implib,libexample.a
    CD ..\..\..\..
    chaos tests\extensions\test.kaos
    IF errorlevel 1 (
        EXIT /B 1
    )
    EXIT /B 0
) ELSE IF [%1]==[test-extensions-windows-clang] (
    CD tests\extensions\spells\example
    clang -shared example.c -o example.o
    clang -c example.c
    clang -shared -o example.dll example.o
    CD ..\..\..\..
    chaos tests\extensions\test.kaos
    IF errorlevel 1 (
        EXIT /B 1
    )
    EXIT /B 0
)

win_flex --wincompat lexer\lexer.l
IF errorlevel 1 (
    EXIT /B 1
)
win_bison -d parser\parser.y
IF errorlevel 1 (
    EXIT /B 1
)
%compiler% -DCHAOS_INTERPRETER -o chaos.exe parser.tab.c lex.yy.c parser/*.c utilities/*.c ast/*.c interpreter/*.c compiler/*.c Chaos.c %extra_flags%
IF errorlevel 1 (
    EXIT /B 1
)

EXIT /B 0

:InstallRequirements
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

:InstallRequirementsDev
CALL RefreshEnv.cmd

gcc -dumpversion > tmpFile
SET /p GCC_VERSION= < tmpFile
DEL tmpFile

clang -dumpversion > tmpFile
SET /p CLANG_VERSION= < tmpFile
DEL tmpFile

ECHO "%programdata%\Chocolatey\lib\mingw\tools\install\mingw64\lib\gcc\x86_64-w64-mingw32\!GCC_VERSION!\include\"
IF not exist "%programdata%\Chocolatey\lib\mingw\tools\install\mingw64\lib\gcc\x86_64-w64-mingw32\!GCC_VERSION!\include\chaos" mkdir "%programdata%\Chocolatey\lib\mingw\tools\install\mingw64\lib\gcc\x86_64-w64-mingw32\!GCC_VERSION!\include\chaos"
COPY utilities\language.h "%programdata%\Chocolatey\lib\mingw\tools\install\mingw64\lib\gcc\x86_64-w64-mingw32\!GCC_VERSION!\include\chaos\"
COPY utilities\platform.h "%programdata%\Chocolatey\lib\mingw\tools\install\mingw64\lib\gcc\x86_64-w64-mingw32\!GCC_VERSION!\include\chaos\"
COPY enums.h "%programdata%\Chocolatey\lib\mingw\tools\install\mingw64\lib\gcc\x86_64-w64-mingw32\!GCC_VERSION!\include\chaos\"
COPY Chaos.h "%programdata%\Chocolatey\lib\mingw\tools\install\mingw64\lib\gcc\x86_64-w64-mingw32\!GCC_VERSION!\include\"
IF errorlevel 1 (
    EXIT /B 1
)

ECHO "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\"
IF not exist "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\chaos" mkdir "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\chaos"
COPY utilities\language.h "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\chaos\"
COPY utilities\platform.h "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\chaos\"
COPY enums.h "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\chaos\"
COPY Chaos.h "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\"
IF errorlevel 1 (
    EXIT /B 1
)
EXIT /B 0

:Uninstall
DEL %windir%\System32\chaos.exe
CALL RefreshEnv.cmd

gcc -dumpversion > tmpFile
SET /p GCC_VERSION= < tmpFile
DEL tmpFile

clang -dumpversion > tmpFile
SET /p CLANG_VERSION= < tmpFile
DEL tmpFile

ECHO "%programdata%\Chocolatey\lib\mingw\tools\install\mingw64\lib\gcc\x86_64-w64-mingw32\!GCC_VERSION!\include\"
DEL Chaos.h "%programdata%\Chocolatey\lib\mingw\tools\install\mingw64\lib\gcc\x86_64-w64-mingw32\!GCC_VERSION!\include\Chaos.h"
RMDIR /s /q "%programdata%\Chocolatey\lib\mingw\tools\install\mingw64\lib\gcc\x86_64-w64-mingw32\!GCC_VERSION!\include\chaos\"
IF errorlevel 1 (
    EXIT /B 1
)

ECHO "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\"
DEL "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\Chaos.h"
RMDIR /s /q "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\chaos\"
IF errorlevel 1 (
    EXIT /B 1
)
EXIT /B 0
