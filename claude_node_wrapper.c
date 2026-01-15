/**
 * Claude Code Direct Node Wrapper for Windows
 *
 * Purpose: Bypasses cmd.exe and calls node.exe + cli.js directly to avoid
 *          command-line parsing issues with complex arguments.
 *
 * This wrapper:
 * 1. Receives arguments from VS Code extension
 * 2. Calls: node.exe <cli.js path> <all arguments>
 * 3. Bypasses cmd.exe to preserve complex JSON/special characters
 *
 * Security: Implements buffer overflow protection and safe process creation.
 *
 * Author: Generated for claude-wrapper project
 * Date: 2026-01-15
 */

#include <windows.h>
#include <stdio.h>
#include <wchar.h>

#define MAX_CMDLINE 32768

// =============================================================================
// CONFIGURATION: Update these paths to match your system
// =============================================================================
// Find your paths:
//   where node     -> Shows NODE_EXE_PATH
//   where claude   -> Parent directory contains node_modules for CLI_JS_PATH
//
// Common locations:
//   Standard npm:  C:\Users\<user>\AppData\Roaming\npm\node_modules\@anthropic-ai\claude-code\cli.js
//   Scoop:         C:\Users\<user>\scoop\persist\nodejs-lts\npm-global\node_modules\@anthropic-ai\claude-code\cli.js
// =============================================================================

#define NODE_EXE_PATH L"C:\\Program Files\\nodejs\\node.exe"
#define CLI_JS_PATH L"C:\\Users\\USERNAME\\AppData\\Roaming\\npm\\node_modules\\@anthropic-ai\\claude-code\\cli.js"

/**
 * Main entry point
 *
 * Calls: node.exe cli.js <args from extension>
 */
int wmain(int argc, wchar_t *argv[]) {
    // Get the full command line (preserves original quoting)
    LPWSTR fullCmd = GetCommandLineW();

    // Skip past our wrapper executable name
    int inQuotes = 0;
    LPWSTR cmdStart = fullCmd;
    while (*cmdStart) {
        if (*cmdStart == L'\"') {
            inQuotes = !inQuotes;
        } else if (*cmdStart == L' ' && !inQuotes) {
            cmdStart++;
            break;
        }
        cmdStart++;
    }

    // Skip leading spaces
    while (*cmdStart == L' ') {
        cmdStart++;
    }

    // Build new command line: node.exe cli.js <original args>
    wchar_t commandLine[MAX_CMDLINE];
    int written = _snwprintf(commandLine, MAX_CMDLINE, L"\"%s\" \"%s\" %s",
                             NODE_EXE_PATH, CLI_JS_PATH, cmdStart);

    if (written < 0 || written >= MAX_CMDLINE) {
        fwprintf(stderr, L"Error: Command line too long\n");
        return 1;
    }

    // Debug output
    #ifdef DEBUG
    wprintf(L"Debug: Executing: %s\n", commandLine);
    #endif

    // Initialize process structures
    STARTUPINFOW si = {0};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    PROCESS_INFORMATION pi = {0};

    // Create the process (directly call node.exe)
    BOOL success = CreateProcessW(
        NODE_EXE_PATH,     // Application name (node.exe)
        commandLine,       // Command line
        NULL,              // Process security attributes
        NULL,              // Thread security attributes
        TRUE,              // Inherit handles
        0,                 // Creation flags
        NULL,              // Environment
        NULL,              // Current directory
        &si,               // Startup info
        &pi                // Process information
    );

    if (!success) {
        DWORD error = GetLastError();
        fwprintf(stderr, L"Error: Failed to launch Node.js (Error code: %lu)\n", error);
        fwprintf(stderr, L"Node path: %s\n", NODE_EXE_PATH);
        fwprintf(stderr, L"CLI path: %s\n", CLI_JS_PATH);

        switch (error) {
            case ERROR_FILE_NOT_FOUND:
                fwprintf(stderr, L"  -> Node.js not found. Check NODE_EXE_PATH.\n");
                break;
            case ERROR_ACCESS_DENIED:
                fwprintf(stderr, L"  -> Access denied.\n");
                break;
            default:
                fwprintf(stderr, L"  -> See: https://learn.microsoft.com/en-us/windows/win32/debug/system-error-codes\n");
                break;
        }
        return 1;
    }

    // Wait for completion
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Get exit code
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    // Cleanup
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return (int)exitCode;
}
