@ECHO OFF
setlocal EnableDelayedExpansion

ECHO "Testing all official spells"

IF not exist "build\" MKDIR build
CD build

FOR %%a in (math string array json) do (
    IF exist %%a\ RD /q /s %%a\
    git clone https://github.com/chaos-lang/%%a.git --depth 1
    CD %%a\
    CALL make.bat
    CALL make.bat test
    CALL make.bat test-compiler
    CALL make.bat clang
    CALL make.bat test
    CALL make.bat test-compiler
    CD ..
)
