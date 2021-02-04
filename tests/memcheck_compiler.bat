@ECHO OFF
setlocal EnableDelayedExpansion

FOR %%f in (*.kaos) do (
    SET filename=%%f
    SET testname=%%~nf

    SET ignore=false
    IF "!filename:~0,5!" == "exit_" SET ignore=true
    IF "!testname!" == "syntax_error" SET ignore=true
    IF "!testname!" == "preemptive" SET ignore=true
    IF !ignore! == true (
        ECHO.
        ECHO Ignoring memcheck for !filename!
    ) ELSE (
        ECHO.
        ECHO.
        ECHO.
        drmemory -no_follow_children -light -exit_code_if_errors 1 -lib_whitelist msvcrt.dll -batch -ignore_kernel -- chaos -c !filename! -o !testname!
        IF errorlevel 1 (
            EXIT /B 1
        )
    )
)

FOR %%f in (*.kaos) do (
    SET filename=%%f
    SET testname=%%~nf

    SET ignore=false
    IF "!filename:~0,5!" == "exit_" SET ignore=true
    IF "!testname!" == "syntax_error" SET ignore=true
    IF "!testname!" == "preemptive" SET ignore=true
    IF !ignore! == true (
        ECHO.
        ECHO Ignoring memcheck for !filename!
    ) ELSE (
        ECHO.
        ECHO.
        ECHO.
        drmemory -no_follow_children -light -exit_code_if_errors 1 -lib_whitelist msvcrt.dll -batch -ignore_kernel -- chaos build\!testname!.exe
        IF errorlevel 1 (
            EXIT /B 1
        )
    )
)
EXIT /B 0
