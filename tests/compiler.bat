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
        chaos -c %%f -o %%~nf
        IF errorlevel 1 (
            ECHO Fail
            EXIT /B 1
        ) ELSE (
            ECHO OK
        )
    )
)

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
        build\!testname!.exe > tmpFile
        TYPE tmpFile
        IF NOT "!testname!" == "string" (
            powershell -Command "(gc tmpFile) -replace '\\', '/' | Out-File -encoding ASCII tmpFile"
        )
        FC tests\!testname!.out tmpFile
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
