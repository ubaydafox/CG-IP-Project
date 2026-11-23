@echo off
title Chrono Run: DN Dash - Build
echo.
echo ============================================================
echo  Chrono Run: DN Dash  ^|  CSE-402 Computer Graphics Lab
echo  Building with MinGW g++  ...
echo ============================================================
echo.

set COMPILER=g++
where g++ >nul 2>nul
if %ERRORLEVEL% neq 0 (
    if exist "C:\MinGW\bin\g++.exe" (
        set COMPILER="C:\MinGW\bin\g++.exe"
    ) else (
        echo [ERROR] g++ not found. Please install MinGW and add it to PATH.
        if "%1" neq "--no-pause" pause
        exit /b 1
    )
)

%COMPILER% -O2 -I"include" -L"lib" src/*.cpp -lfreeglut -lopengl32 -lglu32 -lwinmm -static-libgcc -static-libstdc++ -o ChronoRun.exe

if %ERRORLEVEL% equ 0 (
    echo.
    echo  [SUCCESS]  ChronoRun.exe is ready.
    echo.
) else (
    echo.
    echo  [ERROR]  Compilation failed. See errors above.
    if "%1" neq "--no-pause" pause
    exit /b 1
)
if "%1" neq "--no-pause" pause
