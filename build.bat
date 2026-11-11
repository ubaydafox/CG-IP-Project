@echo off
title Build Chrono Run: DN Dash
echo ========================================================
echo   Compiling Chrono Run: DN Dash (C++ / OpenGL / FreeGLUT)
echo   Course: CSE-402 Computer Graphics & Image Processing Lab
echo ========================================================
echo.

set COMPILER=g++
where g++ >nul 2>nul
if %ERRORLEVEL% neq 0 (
    if exist "C:\Program Files\CodeBlocks\MinGW\bin\g++.exe" (
        set COMPILER="C:\Program Files\CodeBlocks\MinGW\bin\g++.exe"
    ) else if exist "C:\MinGW\bin\g++.exe" (
        set COMPILER="C:\MinGW\bin\g++.exe"
    ) else (
        echo [ERROR] g++ compiler not found in PATH or standard CodeBlocks directories.
        echo Please ensure MinGW/CodeBlocks is installed.
        pause
        exit /b 1
    )
)

echo Using compiler: %COMPILER%
echo Compiling src/main.cpp...

%COMPILER% -O2 -static -I"include" -L"lib" src/main.cpp -lfreeglut -lopengl32 -lglu32 -o ChronoRun.exe

if %ERRORLEVEL% equ 0 (
    echo.
    echo [SUCCESS] Compilation succeeded!
    echo Output executable: ChronoRun.exe
    echo.
) else (
    echo.
    echo [ERROR] Compilation failed. Please review the errors above.
    pause
    exit /b 1
)

pause
