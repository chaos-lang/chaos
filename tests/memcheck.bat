@ECHO OFF
setlocal EnableDelayedExpansion

FOR %%f in (*.kaos) do (
    SET filename=%%f
    SET testname=%%~nf

    SET ignore=false
    IF "!testname!" == "syntax_error" SET ignore=true
    IF !ignore! == true (
        ECHO.
        ECHO Ignoring memcheck for !filename!
    ) ELSE (
        ECHO.
        ECHO.
        ECHO.
        drmemory -light -exit_code_if_errors 1 -lib_whitelist msvcrt.dll -batch -ignore_kernel -- chaos !filename!
        IF errorlevel 1 (
            EXIT /B 1
        )
    )
)
EXIT /B 0
