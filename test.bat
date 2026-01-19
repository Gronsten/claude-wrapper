@echo off
REM Legacy test script for claude-wrapper.exe
REM NOTE: Prefer using: claude-wrapper.exe --wrapper-test
REM
REM This script tests Claude Code passthrough functionality
REM by running various test cases to verify the wrapper works correctly.

echo ========================================
echo Testing Claude Wrapper (Legacy Tests)
echo ========================================
echo.
echo NOTE: You can now use: claude-wrapper.exe --wrapper-test
echo       for a built-in comprehensive test suite.
echo.

REM Check if wrapper exists
if not exist claude-wrapper.exe (
    echo ERROR: claude-wrapper.exe not found
    echo Please run build.bat first
    exit /b 1
)

echo [Test 1] Version check
echo Command: .\claude-wrapper.exe --version
.\claude-wrapper.exe --version
if %ERRORLEVEL% NEQ 0 (
    echo FAILED: Exit code %ERRORLEVEL%
    exit /b 1
)
echo PASSED
echo.

echo [Test 2] Help text
echo Command: .\claude-wrapper.exe --help
.\claude-wrapper.exe --help | findstr "Usage:"
if %ERRORLEVEL% NEQ 0 (
    echo FAILED: Exit code %ERRORLEVEL%
    exit /b 1
)
echo PASSED
echo.

echo [Test 3] Simple calculation with spaces
echo Command: .\claude-wrapper.exe --print "2+2"
.\claude-wrapper.exe --print "2+2" 2>&1 | findstr "4"
if %ERRORLEVEL% NEQ 0 (
    echo FAILED: Exit code %ERRORLEVEL%
    exit /b 1
)
echo PASSED
echo.

echo [Test 4] Multiple arguments
echo Command: .\claude-wrapper.exe --print --model sonnet "test"
.\claude-wrapper.exe --print --model sonnet "test" 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo FAILED: Exit code %ERRORLEVEL%
    exit /b 1
)
echo PASSED
echo.

echo [Test 5] Exit code forwarding (invalid flag)
echo Command: .\claude-wrapper.exe --invalid-flag-xyz
.\claude-wrapper.exe --invalid-flag-xyz >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo FAILED: Should have returned non-zero exit code
    exit /b 1
)
echo PASSED: Non-zero exit code correctly forwarded
echo.

echo ========================================
echo All tests passed!
echo ========================================
echo.
echo The wrapper is working correctly and ready for use.
echo.
echo Next steps:
echo 1. Copy claude-wrapper.exe to a permanent location
echo 2. Update VS Code settings.json with the path
echo.

pause
