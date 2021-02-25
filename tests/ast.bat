@ECHO OFF
setlocal EnableDelayedExpansion

for %%f in (tests\*.kaos) do (
    SET filename=%%f
    SET testname=%%~nf

    SET ignore=false
    IF "!testname!" == "syntax_error" SET ignore=true
    IF !ignore! == true (
        ECHO.
        ECHO Ignoring !filename!
    ) ELSE (
        ECHO.
        ECHO.
        ECHO.
        chaos -a !filename! > tmpFile
        TYPE tmpFile
        FC tests\!testname!.json tmpFile
        IF errorlevel 1 (
            DEL tmpFile
            ECHO Fail
            EXIT /B 1
        ) ELSE (
            DEL tmpFile
            ECHO OK
        )
    )
)
EXIT /B 0
