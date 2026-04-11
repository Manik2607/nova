@echo off
set CMAKE_PATH="C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"

echo Building car_game...
%CMAKE_PATH% --build build --target car_game --config Release

if %ERRORLEVEL% equ 0 (
    echo.
    echo Running car_game...
    ".\build\examples\car_game\Release\car_game.exe"
) else (
    echo.
    echo Build FAILED!
    pause
)
