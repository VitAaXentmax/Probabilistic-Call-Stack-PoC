@echo off
REM Probabilistic Call Stack PoC - Build Script for Windows
REM Run from Visual Studio Developer Command Prompt for MSVC
REM Or use with MinGW installed

echo ==========================================
echo Probabilistic Call Stack PoC - Build
echo ==========================================
echo.

REM Check for MSVC
where cl >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo [BUILD] Using MSVC compiler
    cl /EHsc /O2 /W3 /nologo probabilistic_callstack.cpp /Fe:probabilistic_callstack.exe /link user32.lib dbghelp.lib
    goto :done
)

REM Check for MinGW
where g++ >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo [BUILD] Using MinGW compiler
    g++ -O2 -Wall -static probabilistic_callstack.cpp -o probabilistic_callstack.exe -luser32 -ldbghelp
    goto :done
)

echo [ERROR] No compiler found!
echo Please run from Visual Studio Developer Command Prompt
echo or ensure MinGW is in your PATH
exit /b 1

:done
echo.
if exist probabilistic_callstack.exe (
    echo [SUCCESS] Build complete: probabilistic_callstack.exe
    echo.
    echo Run with: probabilistic_callstack.exe [num_runs]
    echo   num_runs: Number of executions (1-10, default: 3)
) else (
    echo [ERROR] Build failed
)
