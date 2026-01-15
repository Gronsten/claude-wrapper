@echo off
REM Build script for claude-wrapper.exe
REM Requires: MinGW GCC (gcc.exe in PATH)
REM
REM This wrapper calls Node.js directly to bypass the buggy Bun executable
REM bundled with Claude Code VS Code extension 2.1.7+

echo ========================================
echo Building Claude Wrapper for Windows
echo ========================================
echo.

REM Check if gcc is available
where gcc >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: gcc not found in PATH
    echo.
    echo Please install MinGW or ensure gcc.exe is in your PATH
    echo You can install via Scoop: scoop install gcc
    echo.
    pause
    exit /b 1
)

echo [1/3] Checking gcc version...
gcc --version | findstr "gcc"
echo.

echo [2/3] Compiling claude_node_wrapper.c...
gcc -o claude-wrapper.exe claude_node_wrapper.c -municode -mconsole -O2 -s
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Compilation failed
    pause
    exit /b 1
)
echo       Successfully compiled: claude-wrapper.exe
echo.

echo [3/3] Verifying output...
if exist claude-wrapper.exe (
    echo       Output file exists: claude-wrapper.exe
    for %%A in (claude-wrapper.exe) do echo       File size: %%~zA bytes
    echo.
    echo ========================================
    echo Build completed successfully!
    echo ========================================
    echo.
    echo To test the wrapper:
    echo   claude-wrapper.exe --version
    echo.
    echo To install for VS Code:
    echo   1. Copy claude-wrapper.exe to a permanent location
    echo   2. Update VS Code settings.json:
    echo      "claudeCode.claudeProcessWrapper": "C:\\path\\to\\claude-wrapper.exe"
    echo.
) else (
    echo ERROR: Output file not created
    exit /b 1
)

pause
