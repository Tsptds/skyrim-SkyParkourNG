cmake -B build -S . --preset default --fresh
cmake --build build --preset release
@echo off
echo.
set /p DUMMY=Hit Enter to exit...