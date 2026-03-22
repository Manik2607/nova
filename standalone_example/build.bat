@echo off
setlocal enabledelayedexpansion

:: --- CONFIGURATION ---
set NOVA_ROOT=..\NovaGFX
set BUILD_DIR=%NOVA_ROOT%\build_mingw

:: Compiler/Linker Flags
set CXX=g++
set CXX_FLAGS=-std=c++20 -O2 -Wall -Wextra
set INCLUDES=-Istandalone_example\player.hpp -I%NOVA_ROOT%\include -I%BUILD_DIR%\_deps\glfw-src\include -I%BUILD_DIR%\_deps\glad-build\include -I%BUILD_DIR%\_deps\box2d-src\include -I%BUILD_DIR%\_deps\box2d-build\include -I%BUILD_DIR%\_deps\stb-src
set LIB_DIRS=-L%BUILD_DIR% -L%BUILD_DIR%\_deps\glfw-build\src -L%BUILD_DIR%\_deps\glad-build -L%BUILD_DIR%\_deps\box2d-build\bin
set LIBS=-lnova -lglfw3 -lglad -lbox2d -lopengl32 -lgdi32 -lwinmm

:: --- BUILD ---
echo [BUILD] Compiling main.cpp...
%CXX% %CXX_FLAGS% %INCLUDES% player.cpp main.cpp -o game.exe %LIB_DIRS% %LIBS%

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Build failed^!
    exit /b %ERRORLEVEL%
)

echo [SUCCESS] Build complete. Running game.exe...
.\game.exe
