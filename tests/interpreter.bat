@ECHO OFF
setlocal EnableDelayedExpansion

for %%f in (tests\*.kaos) do (
    ECHO.
    ECHO.
    ECHO.
    chaos %%f > tmpFile
    TYPE tmpFile
    powershell -Command "(gc tmpFile) -replace '\\', '/' | Out-File -encoding ASCII tmpFile"
    FC tests\%%~nf.out tmpFile
    IF errorlevel 1 (
        DEL tmpFile
        ECHO Fail
        EXIT /B 1
    ) ELSE (
        DEL tmpFile
        ECHO OK
    )
)
EXIT /B 0
