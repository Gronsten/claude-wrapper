# Claude Wrapper Architecture

Technical documentation for understanding how the claude-wrapper project works.

## Overview

This project provides a native Windows executable wrapper that allows the Claude Code VS Code extension to use the stable Node.js-based CLI instead of the buggy bundled Bun executable.

## Problem Background

### The Bun Crash Issue

Claude Code VS Code extension version 2.1.7+ bundles a standalone Bun executable that crashes on some Windows systems:

```
panic(main thread): Segmentation fault at address 0x113
oh no: Bun has crashed. This indicates a bug in Bun, not your code.
```

The crash occurs in `ctiuser.dll` (Windows UI library), indicating a Windows-specific Bun compatibility issue.

### Why a Wrapper?

The VS Code extension provides a `claudeCode.claudeProcessWrapper` setting that allows specifying an alternative executable. However:

1. The extension expects a `.exe` file, not a `.cmd` batch file
2. Using `cmd.exe` as an intermediary mangles complex JSON arguments
3. The wrapper must handle VS Code's complex command-line arguments (JSON, system prompts, special characters)

## Architecture

### Execution Flow

```
┌─────────────────────┐
│  VS Code Extension  │
│  (Claude Code)      │
└──────────┬──────────┘
           │ Calls with args:
           │ --output-format stream-json
           │ --append-system-prompt "..."
           │ --model default
           │ etc.
           ▼
┌─────────────────────┐
│  claude-wrapper.exe │
│  (This project)     │
└──────────┬──────────┘
           │ Forwards all args to:
           ▼
┌─────────────────────┐
│     node.exe        │
│  + cli.js script    │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│   Claude API        │
└─────────────────────┘
```

### Key Design Decisions

#### 1. Direct Node.js Invocation

**Why:** Bypasses `cmd.exe` which mangles complex JSON arguments.

**How:** Uses `CreateProcessW` with explicit `node.exe` path as application name.

```c
CreateProcessW(
    NODE_EXE_PATH,     // "C:\...\node.exe"
    commandLine,       // "node.exe" "cli.js" <args>
    ...
);
```

#### 2. Preserve Original Command Line

**Why:** VS Code passes carefully quoted arguments that must not be modified.

**How:** Uses `GetCommandLineW()` to get the original command line, then strips our wrapper's path and forwards everything else unchanged.

```c
LPWSTR fullCmd = GetCommandLineW();
// Skip past wrapper.exe name
// Forward everything after to node.exe
```

#### 3. Handle Inheritance

**Why:** The extension communicates via stdin/stdout streams.

**How:** Inherits standard handles and passes them to the child process.

```c
si.dwFlags = STARTF_USESTDHANDLES;
si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
```

#### 4. Exit Code Forwarding

**Why:** The extension checks exit codes to detect failures.

**How:** Waits for child process and returns its exit code.

```c
WaitForSingleObject(pi.hProcess, INFINITE);
GetExitCodeProcess(pi.hProcess, &exitCode);
return (int)exitCode;
```

## File Structure

```
claude-wrapper/
├── claude_node_wrapper.c   # Main source code
├── build.bat               # Build script
├── test.bat                # Test suite
├── README.md               # User documentation
├── INSTALL.md              # Installation guide
├── QUICKSTART.md           # Quick reference
├── ARCHITECTURE.md         # This file
├── .gitignore              # Git ignore rules
└── claude-wrapper.exe      # Compiled output (git-ignored)
```

## Source Code Reference

### claude_node_wrapper.c

| Section | Lines | Description |
|---------|-------|-------------|
| Includes | 1-15 | Windows headers, defines |
| Path Constants | 17-19 | `NODE_EXE_PATH`, `CLI_JS_PATH` |
| wmain() | 27-115 | Main entry point |
| - Arg parsing | 32-59 | Extract args from command line |
| - Command build | 51-58 | Build new command line |
| - Process spawn | 68-95 | CreateProcessW call |
| - Wait/cleanup | 98-112 | Wait and return exit code |

### Hardcoded Paths

These paths must be updated to match your system before building:

```c
// Standard Node.js installation
#define NODE_EXE_PATH L"C:\\Program Files\\nodejs\\node.exe"
#define CLI_JS_PATH L"C:\\Users\\YourName\\AppData\\Roaming\\npm\\node_modules\\@anthropic-ai\\claude-code\\cli.js"

// Scoop installation
#define NODE_EXE_PATH L"C:\\Users\\YourName\\scoop\\apps\\nodejs-lts\\current\\node.exe"
#define CLI_JS_PATH L"C:\\Users\\YourName\\scoop\\persist\\nodejs-lts\\npm-global\\node_modules\\@anthropic-ai\\claude-code\\cli.js"
```

## Failed Approaches

### Attempt 1: cmd.exe Wrapper

```c
// FAILED - cmd.exe mangles JSON arguments
commandLine = L"cmd /c claude.cmd <args>";
```

**Result:** "The filename, directory name, or volume label syntax is incorrect"

### Attempt 2: Transparent Pass-Through

```c
// FAILED - Extension still used bundled Bun
CreateProcessW(NULL, fullCommandLine, ...);
```

**Result:** Bun still crashed because extension's own binary was invoked.

### Attempt 3: Direct Node.js (SUCCESS)

```c
// SUCCESS - Bypasses both cmd.exe and Bun
CreateProcessW(NODE_EXE_PATH, "node.exe cli.js <args>", ...);
```

**Result:** Works with both extension 2.0.28 and 2.1.7+.

## Security Considerations

### Buffer Overflow Protection

- Uses `_snwprintf` with size limit
- Checks return value for truncation
- Maximum command line: 32KB (Windows limit)

### Process Creation

- Explicit application path (no PATH search injection)
- No shell interpretation of arguments
- Direct argument forwarding

### Trust Boundary

- Wrapper adds no new attack surface
- All security delegated to Node.js and Claude CLI
- Arguments passed unchanged

## Build System

### Requirements

- MinGW GCC (tested with 15.2.0)
- Windows 10/11

### Compilation Flags

```batch
gcc -o claude-wrapper.exe claude_node_wrapper.c -municode -mconsole -O2 -s
```

| Flag | Purpose |
|------|---------|
| `-municode` | Use Unicode entry point (wmain) |
| `-mconsole` | Console application subsystem |
| `-O2` | Optimization level 2 |
| `-s` | Strip debug symbols |

### Debug Build

```batch
gcc -o claude-wrapper.exe claude_node_wrapper.c -municode -mconsole -DDEBUG
```

Enables debug output showing the command being executed.

## Testing

### Test Suite (test.bat)

1. **Version check** - Basic functionality
2. **Help text** - Argument passing
3. **Simple calculation** - API communication
4. **Multiple arguments** - Complex arg handling
5. **Exit code forwarding** - Error propagation

### Manual Testing

```batch
# Basic test
claude-wrapper.exe --version

# Complex arguments (simulates VS Code)
claude-wrapper.exe --output-format stream-json --append-system-prompt "{\"test\": true}" --print "Hello"
```

## Compatibility Matrix

| Component | Version | Status |
|-----------|---------|--------|
| Windows | 10/11 | ✅ Tested |
| Node.js | 24.13.0 | ✅ Tested |
| Claude CLI | 2.1.7 | ✅ Tested |
| VS Code Extension | 2.0.28 | ✅ Works |
| VS Code Extension | 2.1.7+ | ✅ Works (bypasses Bun) |
| GCC | 15.2.0 | ✅ Tested |

## Future Considerations

### Path Configuration

Currently paths are hardcoded. Future improvements could:
- Read paths from environment variables
- Use a configuration file
- Auto-detect Node.js location

### Extension Updates

If Anthropic fixes the Bun crash or changes the `claudeProcessWrapper` behavior, the wrapper may need updates.

### Node.js Updates

Scoop's `current` symlink should handle Node.js updates automatically. If the symlink changes location, update `NODE_EXE_PATH`.
