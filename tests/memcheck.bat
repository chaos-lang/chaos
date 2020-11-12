@ECHO OFF
setlocal EnableDelayedExpansion

FOR %%f in (*.kaos) do (
    set filename=%%f

    IF "!filename:~0,5!"=="exit_" (
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
