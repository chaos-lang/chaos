@ECHO OFF
setlocal enabledelayedexpansion
for %%f in (*.kaos) do (
    drmemory -light -exit_code_if_errors 1 -lib_whitelist msvcrt.dll -batch -ignore_kernel -- chaos %%f
    IF errorlevel 1 (
        EXIT /B 1
    )
)
