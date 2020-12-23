@ECHO OFF
setlocal EnableDelayedExpansion

ECHO "Installing all official spells"

CD tests\rosetta
IF not exist "spells\" MKDIR spells
CD spells

FOR %%a in (math string array json) do (
    IF exist %%a\ DEL %%a\
    git clone https://github.com/chaos-lang/%%a.git --depth 1
    CD %%a\
    CALL make.bat
    CD ..
)
