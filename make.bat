@ECHO OFF
setlocal EnableDelayedExpansion

IF [%1]==[] (
    SET compiler=gcc
    SET extra_flags=-D__USE_MINGW_ANSI_STDIO -Werror -Wall -pedantic -Wl,--stack,4294967296
) ELSE IF [%1]==[dev] (
    SET compiler=gcc
    SET extra_flags=-D__USE_MINGW_ANSI_STDIO -Werror -Wall -pedantic -Wl,--stack,4294967296 -ggdb
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
) ELSE IF [%1]==[test-compiler] (
    CALL tests\compiler.bat
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
) ELSE IF [%1]==[memcheck-compiler] (
    CD tests
    CALL memcheck_compiler.bat
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
) ELSE IF [%1]==[test-compiler-extensions-windows-gcc] (
    CALL make.bat test-extensions-windows-gcc
    chaos -c tests\extensions\test.kaos
    build\main
    IF errorlevel 1 (
        EXIT /B 1
    )
    EXIT /B 0
) ELSE IF [%1]==[test-compiler-extensions-windows-clang] (
    CALL make.bat test-extensions-windows-clang
    chaos -c tests\extensions\test.kaos
    build\main
    IF errorlevel 1 (
        EXIT /B 1
    )
    EXIT /B 0
) ELSE IF [%1]==[test-official-spells] (
    CALL tests\official_spells.bat
    IF errorlevel 1 (
        EXIT /B 1
    ) ELSE (
        EXIT /B 0
    )
) ELSE IF [%1]==[rosetta-install] (
    CALL tests\rosetta\install.bat
    IF errorlevel 1 (
        EXIT /B 1
    ) ELSE (
        EXIT /B 0
    )
) ELSE IF [%1]==[rosetta-install-clang] (
    CALL tests\rosetta\install.bat clang
    IF errorlevel 1 (
        EXIT /B 1
    ) ELSE (
        EXIT /B 0
    )
) ELSE IF [%1]==[rosetta-test] (
    CALL tests\rosetta\interpreter.bat
    IF errorlevel 1 (
        EXIT /B 1
    ) ELSE (
        EXIT /B 0
    )
) ELSE IF [%1]==[rosetta-test-compiler] (
    CALL tests\rosetta\compiler.bat
    IF errorlevel 1 (
        EXIT /B 1
    ) ELSE (
        EXIT /B 0
    )
)

win_flex --wincompat lexer\lexer.l
IF errorlevel 1 (
    EXIT /B 1
)
win_bison -d parser\parser.y
IF errorlevel 1 (
    EXIT /B 1
)
%compiler% -fcommon -DCHAOS_INTERPRETER -o chaos.exe parser.tab.c lex.yy.c parser/*.c utilities/*.c utilities/windows/*.c ast/*.c interpreter/*.c compiler/*.c Chaos.c %extra_flags%
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
choco install llvm --confirm --version 10.0.0
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
ECHO D | XCOPY /K /D /H /Y utilities "%programdata%\Chocolatey\lib\mingw\tools\install\mingw64\lib\gcc\x86_64-w64-mingw32\!GCC_VERSION!\include\chaos\utilities"
ECHO D | XCOPY /K /D /H /Y utilities\windows "%programdata%\Chocolatey\lib\mingw\tools\install\mingw64\lib\gcc\x86_64-w64-mingw32\!GCC_VERSION!\include\chaos\utilities\windows"
ECHO D | XCOPY /K /D /H /Y lexer "%programdata%\Chocolatey\lib\mingw\tools\install\mingw64\lib\gcc\x86_64-w64-mingw32\!GCC_VERSION!\include\chaos\lexer"
ECHO D | XCOPY /K /D /H /Y parser "%programdata%\Chocolatey\lib\mingw\tools\install\mingw64\lib\gcc\x86_64-w64-mingw32\!GCC_VERSION!\include\chaos\parser"
ECHO D | XCOPY /K /D /H /Y interpreter "%programdata%\Chocolatey\lib\mingw\tools\install\mingw64\lib\gcc\x86_64-w64-mingw32\!GCC_VERSION!\include\chaos\interpreter"
ECHO D | XCOPY /K /D /H /Y compiler "%programdata%\Chocolatey\lib\mingw\tools\install\mingw64\lib\gcc\x86_64-w64-mingw32\!GCC_VERSION!\include\chaos\compiler"
ECHO D | XCOPY /K /D /H /Y compiler\lib "%programdata%\Chocolatey\lib\mingw\tools\install\mingw64\lib\gcc\x86_64-w64-mingw32\!GCC_VERSION!\include\chaos\compiler\lib"
ECHO D | XCOPY /K /D /H /Y ast "%programdata%\Chocolatey\lib\mingw\tools\install\mingw64\lib\gcc\x86_64-w64-mingw32\!GCC_VERSION!\include\chaos\ast"
COPY lex.yy.c "%programdata%\Chocolatey\lib\mingw\tools\install\mingw64\lib\gcc\x86_64-w64-mingw32\!GCC_VERSION!\include\chaos\"
COPY parser.tab.h "%programdata%\Chocolatey\lib\mingw\tools\install\mingw64\lib\gcc\x86_64-w64-mingw32\!GCC_VERSION!\include\chaos\"
COPY parser.tab.c "%programdata%\Chocolatey\lib\mingw\tools\install\mingw64\lib\gcc\x86_64-w64-mingw32\!GCC_VERSION!\include\chaos\"
COPY enums.h "%programdata%\Chocolatey\lib\mingw\tools\install\mingw64\lib\gcc\x86_64-w64-mingw32\!GCC_VERSION!\include\chaos\"
COPY Chaos.c "%programdata%\Chocolatey\lib\mingw\tools\install\mingw64\lib\gcc\x86_64-w64-mingw32\!GCC_VERSION!\include\chaos\"
COPY Chaos.h "%programdata%\Chocolatey\lib\mingw\tools\install\mingw64\lib\gcc\x86_64-w64-mingw32\!GCC_VERSION!\include\"
COPY LICENSE "%programdata%\Chocolatey\lib\mingw\tools\install\mingw64\lib\gcc\x86_64-w64-mingw32\!GCC_VERSION!\include\chaos\"
IF errorlevel 1 (
    EXIT /B 1
)

ECHO "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\"
IF not exist "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\chaos" mkdir "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\chaos"
ECHO D | XCOPY /K /D /H /Y utilities "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\chaos\utilities"
ECHO D | XCOPY /K /D /H /Y utilities\windows "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\chaos\utilities\windows"
ECHO D | XCOPY /K /D /H /Y lexer "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\chaos\lexer"
ECHO D | XCOPY /K /D /H /Y parser "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\chaos\parser"
ECHO D | XCOPY /K /D /H /Y interpreter "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\chaos\interpreter"
ECHO D | XCOPY /K /D /H /Y compiler "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\chaos\compiler"
ECHO D | XCOPY /K /D /H /Y compiler\lib "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\chaos\compiler\lib"
ECHO D | XCOPY /K /D /H /Y ast "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\chaos\ast"
COPY lex.yy.c "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\chaos\"
COPY parser.tab.h "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\chaos\"
COPY parser.tab.c "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\chaos\"
COPY enums.h "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\chaos\"
COPY Chaos.c "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\chaos\"
COPY Chaos.h "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\"
COPY LICENSE "%programfiles%\LLVM\lib\clang\!CLANG_VERSION!\include\chaos\"
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
