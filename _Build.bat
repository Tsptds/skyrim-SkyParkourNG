cmake -B build -S . --preset no-vr --fresh
cmake --build build --preset release-novr
@echo off
echo.
set /p DUMMY=Hit Enter to exit...