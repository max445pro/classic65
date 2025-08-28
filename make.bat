@echo off
setlocal EnableDelayedExpansion

:: Name of the output executable
set OUTPUT=c64.exe

:: List of all .c source files
set SOURCES=main.c src\lib6502.c
:: Compiler flags (you can add -Wall -O2 etc.)
:: set CFLAGS=

:: Linker flags (libraries to link)
:: set LIBS=

:: Build

echo Compiling...
gcc %CFLAGS% %SOURCES% %LIBS% -o %OUTPUT%

if errorlevel 1 (
    echo Build failed.
    exit /b 1
)

echo Build succeeded. Output: %OUTPUT%
