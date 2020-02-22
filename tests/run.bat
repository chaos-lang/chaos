@ECHO OFF
setlocal enabledelayedexpansion
for %%f in (*.kaos) do (
    chaos %%f > tmpFile
    TYPE tmpFile
    FC %%~nf.out tmpFile
    IF errorlevel 1 (
        ECHO Fail
        EXIT /B 1
    ) ELSE (
        ECHO OK
    )
    DEL tmpFile
)