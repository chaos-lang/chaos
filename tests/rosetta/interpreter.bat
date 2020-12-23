@ECHO OFF
setlocal EnableDelayedExpansion

for %%f in (tests\rosetta\*.kaos) do (
    ECHO.
    ECHO.
    ECHO.
    chaos %%f > tmpFile
    TYPE tmpFile
    IF NOT "%%~nf" == "string" (
        powershell -Command "(gc tmpFile) -replace '\\', '/' | Out-File -encoding ASCII tmpFile"
    )
    FC tests\rosetta\%%~nf.out tmpFile
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
