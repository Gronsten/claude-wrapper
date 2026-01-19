/**
 * Claude Code Direct Node Wrapper for Windows
 *
 * Purpose: Bypasses cmd.exe and calls node.exe + cli.js directly to avoid
 *          command-line parsing issues with complex arguments.
 *
 * This wrapper:
 * 1. Loads configuration from claude-wrapper.json (same directory as .exe)
 * 2. Handles wrapper-specific CLI flags (--wrapper-version, --wrapper-help, --wrapper-test)
 * 3. Receives arguments from VS Code extension
 * 4. Calls: node.exe <cli.js path> <all arguments>
 * 5. Bypasses cmd.exe to preserve complex JSON/special characters
 *
 * Security: Implements buffer overflow protection and safe process creation.
 *
 * Author: Generated for claude-wrapper project
 * Date: 2026-01-15
 * Updated: 2026-01-19 (Added wrapper-specific CLI switches)
 */

#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

#define MAX_CMDLINE 32768
#define MAX_PATH_LEN 1024
#define CONFIG_FILENAME L"claude-wrapper.json"
#define WRAPPER_VERSION L"1.2.0"

// Default paths (used if config file not found)
#define DEFAULT_NODE_PATH L"C:\\Program Files\\nodejs\\node.exe"
#define DEFAULT_CLI_PATH L"C:\\Users\\USERNAME\\AppData\\Roaming\\npm\\node_modules\\@anthropic-ai\\claude-code\\cli.js"

// Global path buffers
static wchar_t g_nodePath[MAX_PATH_LEN];
static wchar_t g_cliPath[MAX_PATH_LEN];

/**
 * Skip whitespace in a string
 */
static const char* skipWhitespace(const char* s) {
    while (*s && (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r')) s++;
    return s;
}

/**
 * Extract a JSON string value for a given key
 * Simple parser - handles basic JSON with string values only
 * Returns 1 on success, 0 on failure
 */
static int extractJsonString(const char* json, const char* key, wchar_t* outValue, size_t maxLen) {
    char searchKey[256];
    _snprintf(searchKey, sizeof(searchKey), "\"%s\"", key);

    const char* keyPos = strstr(json, searchKey);
    if (!keyPos) return 0;

    // Skip past key and find colon
    keyPos += strlen(searchKey);
    keyPos = skipWhitespace(keyPos);
    if (*keyPos != ':') return 0;
    keyPos++;
    keyPos = skipWhitespace(keyPos);

    // Expect opening quote
    if (*keyPos != '"') return 0;
    keyPos++;

    // Extract value until closing quote (handle escape sequences)
    char tempValue[MAX_PATH_LEN];
    size_t i = 0;
    while (*keyPos && *keyPos != '"' && i < sizeof(tempValue) - 1) {
        if (*keyPos == '\\' && *(keyPos + 1)) {
            keyPos++; // Skip backslash
            switch (*keyPos) {
                case '\\': tempValue[i++] = '\\'; break;
                case '"': tempValue[i++] = '"'; break;
                case 'n': tempValue[i++] = '\n'; break;
                case 't': tempValue[i++] = '\t'; break;
                default: tempValue[i++] = *keyPos; break;
            }
        } else {
            tempValue[i++] = *keyPos;
        }
        keyPos++;
    }
    tempValue[i] = '\0';

    // Convert to wide string
    if (MultiByteToWideChar(CP_UTF8, 0, tempValue, -1, outValue, (int)maxLen) == 0) {
        return 0;
    }

    return 1;
}

/**
 * Load configuration from JSON file
 * Returns 1 if config loaded successfully, 0 if using defaults
 */
static int loadConfig(void) {
    // Get path to executable
    wchar_t exePath[MAX_PATH_LEN];
    if (GetModuleFileNameW(NULL, exePath, MAX_PATH_LEN) == 0) {
        return 0;
    }

    // Build path to config file (same directory as exe)
    wchar_t configPath[MAX_PATH_LEN];
    wcscpy(configPath, exePath);
    PathRemoveFileSpecW(configPath);
    PathAppendW(configPath, CONFIG_FILENAME);

    // Open config file
    HANDLE hFile = CreateFileW(configPath, GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        #ifdef DEBUG
        wprintf(L"Debug: Config file not found: %s\n", configPath);
        #endif
        return 0;
    }

    // Read file content
    char buffer[8192] = {0};
    DWORD bytesRead = 0;
    if (!ReadFile(hFile, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
        CloseHandle(hFile);
        return 0;
    }
    CloseHandle(hFile);
    buffer[bytesRead] = '\0';

    #ifdef DEBUG
    wprintf(L"Debug: Loaded config from: %s\n", configPath);
    #endif

    // Parse JSON values
    int nodeFound = extractJsonString(buffer, "nodePath", g_nodePath, MAX_PATH_LEN);
    int cliFound = extractJsonString(buffer, "cliPath", g_cliPath, MAX_PATH_LEN);

    return (nodeFound && cliFound);
}

/**
 * Initialize paths from config file or defaults
 */
static void initializePaths(void) {
    // Try to load from config file
    if (!loadConfig()) {
        // Use defaults
        wcscpy(g_nodePath, DEFAULT_NODE_PATH);
        wcscpy(g_cliPath, DEFAULT_CLI_PATH);

        #ifdef DEBUG
        wprintf(L"Debug: Using default paths\n");
        #endif
    }
}

/**
 * Display wrapper version information
 */
static void showWrapperVersion(void) {
    wprintf(L"claude-wrapper version %s\n", WRAPPER_VERSION);
    wprintf(L"Windows native wrapper for Claude Code CLI\n");
    wprintf(L"https://github.com/Gronsten/claude-wrapper\n");
}

/**
 * Display wrapper help information
 */
static void showWrapperHelp(void) {
    wprintf(L"claude-wrapper - Windows native wrapper for Claude Code CLI\n\n");
    wprintf(L"WRAPPER-SPECIFIC OPTIONS:\n");
    wprintf(L"  --wrapper-version    Show wrapper version and exit\n");
    wprintf(L"  --wrapper-help       Show this wrapper help and exit\n");
    wprintf(L"  --wrapper-test       Test Claude Code passthrough and exit\n\n");
    wprintf(L"All other options are passed through to the Claude Code CLI.\n");
    wprintf(L"For Claude Code CLI help, use: claude --help\n\n");
    wprintf(L"CONFIGURATION:\n");
    wprintf(L"  Config file: claude-wrapper.json (same directory as .exe)\n");
    wprintf(L"  Required fields: nodePath, cliPath\n\n");
    wprintf(L"EXAMPLE USAGE:\n");
    wprintf(L"  claude-wrapper --wrapper-version    # Show wrapper version\n");
    wprintf(L"  claude-wrapper --wrapper-test       # Test passthrough\n");
    wprintf(L"  claude-wrapper --version            # Pass to Claude Code CLI\n");
    wprintf(L"  claude-wrapper --help               # Pass to Claude Code CLI\n");
}

/**
 * Test Claude Code passthrough
 * Returns 0 on success, 1 on failure
 */
static int testPassthrough(void) {
    wprintf(L"Testing Claude Code passthrough...\n\n");

    // Check if config file exists
    wprintf(L"[1/4] Checking configuration...\n");
    if (loadConfig()) {
        wprintf(L"      \u2713 Config file loaded successfully\n");
        wprintf(L"      Node.js: %s\n", g_nodePath);
        wprintf(L"      CLI: %s\n", g_cliPath);
    } else {
        wprintf(L"      ! Config file not found, using defaults\n");
        wcscpy(g_nodePath, DEFAULT_NODE_PATH);
        wcscpy(g_cliPath, DEFAULT_CLI_PATH);
        wprintf(L"      Node.js: %s\n", g_nodePath);
        wprintf(L"      CLI: %s\n", g_cliPath);
    }

    // Check if Node.js exists
    wprintf(L"\n[2/4] Checking Node.js...\n");
    if (GetFileAttributesW(g_nodePath) != INVALID_FILE_ATTRIBUTES) {
        wprintf(L"      \u2713 Node.js found\n");
    } else {
        wprintf(L"      \u2717 Node.js NOT found at: %s\n", g_nodePath);
        wprintf(L"      Please install Node.js or update claude-wrapper.json\n");
        return 1;
    }

    // Check if CLI exists
    wprintf(L"\n[3/4] Checking Claude Code CLI...\n");
    if (GetFileAttributesW(g_cliPath) != INVALID_FILE_ATTRIBUTES) {
        wprintf(L"      \u2713 Claude Code CLI found\n");
    } else {
        wprintf(L"      \u2717 Claude Code CLI NOT found at: %s\n", g_cliPath);
        wprintf(L"      Please run: npm install -g @anthropic-ai/claude-code\n");
        return 1;
    }

    // Test actual passthrough with --version
    wprintf(L"\n[4/4] Testing passthrough (claude --version)...\n");
    wchar_t commandLine[MAX_CMDLINE];
    _snwprintf(commandLine, MAX_CMDLINE, L"\"%s\" \"%s\" --version", g_nodePath, g_cliPath);

    STARTUPINFOW si = {0};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    PROCESS_INFORMATION pi = {0};

    BOOL success = CreateProcessW(g_nodePath, commandLine, NULL, NULL, TRUE,
                                   0, NULL, NULL, &si, &pi);

    if (!success) {
        wprintf(L"      \u2717 Failed to execute Claude Code CLI\n");
        return 1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (exitCode == 0) {
        wprintf(L"\n      \u2713 Passthrough test PASSED\n");
        wprintf(L"\nAll tests passed! The wrapper is working correctly.\n");
        return 0;
    } else {
        wprintf(L"\n      \u2717 Claude Code CLI returned exit code: %lu\n", exitCode);
        return 1;
    }
}

/**
 * Main entry point
 *
 * Calls: node.exe cli.js <args from extension>
 */
int wmain(int argc, wchar_t *argv[]) {
    // Check for wrapper-specific flags first
    if (argc >= 2) {
        if (wcscmp(argv[1], L"--wrapper-version") == 0) {
            showWrapperVersion();
            return 0;
        }
        if (wcscmp(argv[1], L"--wrapper-help") == 0) {
            showWrapperHelp();
            return 0;
        }
        if (wcscmp(argv[1], L"--wrapper-test") == 0) {
            initializePaths();
            return testPassthrough();
        }
    }

    // Initialize paths from config or defaults
    initializePaths();

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
                             g_nodePath, g_cliPath, cmdStart);

    if (written < 0 || written >= MAX_CMDLINE) {
        fwprintf(stderr, L"Error: Command line too long\n");
        return 1;
    }

    // Debug output
    #ifdef DEBUG
    wprintf(L"Debug: Node path: %s\n", g_nodePath);
    wprintf(L"Debug: CLI path: %s\n", g_cliPath);
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
        g_nodePath,        // Application name (node.exe)
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
        fwprintf(stderr, L"Node path: %s\n", g_nodePath);
        fwprintf(stderr, L"CLI path: %s\n", g_cliPath);

        switch (error) {
            case ERROR_FILE_NOT_FOUND:
                fwprintf(stderr, L"  -> Node.js not found. Check nodePath in claude-wrapper.json.\n");
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
