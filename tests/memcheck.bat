@ECHO OFF
setlocal EnableDelayedExpansion

FOR %%f in (*.kaos) do (
    set testname=%%f

    IF "!testname:~0,5!"=="exit_" (
        ECHO.
        ECHO Ignoring memcheck for !testname!
    ) ELSE (
        ECHO.
        ECHO.
        ECHO.
        drmemory -light -exit_code_if_errors 1 -lib_whitelist msvcrt.dll -batch -ignore_kernel -- chaos !testname!
        IF errorlevel 1 (
            EXIT /B 1
        )
    )
)
EXIT /B 0
