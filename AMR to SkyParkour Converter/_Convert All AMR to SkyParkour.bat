@echo off
setlocal

set SCRIPT=_converter.py
set FOLDER=_Annotations

for %%f in ("%FOLDER%\*.txt") do (
    python "%SCRIPT%" "%%f"
)

endlocal
