@ECHO OFF
setlocal enabledelayedexpansion
for %%f in (tests\*.kaos) do (
    ECHO.
    ECHO.
    ECHO.
    chaos %%f > tmpFile
    TYPE tmpFile
    FC tests\%%~nf.out tmpFile
    IF errorlevel 1 (
        ECHO Fail
        EXIT /B 1
    ) ELSE (
        ECHO OK
    )
    DEL tmpFile
)