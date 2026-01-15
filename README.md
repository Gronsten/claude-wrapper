# Claude CLI Wrapper for Windows

A secure native Windows executable wrapper that enables the Claude Code VS Code extension to use the stable Node.js-based CLI instead of the buggy bundled Bun executable.

## Problem Statement

Claude Code VS Code extension version 2.1.7+ bundles a standalone Bun executable (`claude.exe`) that crashes with a segmentation fault on some Windows systems:

```
panic(main thread): Segmentation fault at address 0x113
oh no: Bun has crashed. This indicates a bug in Bun, not your code.
```

This wrapper solves the problem by intercepting the extension's calls and routing them to the stable Node.js-based CLI installed via npm.

## How It Works

**Without wrapper (extension 2.1.7+):**
```
Extension → Bundled claude.exe (Bun) → CRASH
```

**With wrapper:**
```
Extension → claude-wrapper.exe → node.exe + cli.js → SUCCESS
```

The wrapper:
1. Receives all arguments from the VS Code extension
2. Calls `node.exe` directly with the `cli.js` script path
3. Passes all arguments unchanged (preserving JSON, special characters, etc.)
4. Returns the exit code to the extension

## Security Features

- Buffer overflow protection (32KB command line limit check)
- Direct process creation (bypasses cmd.exe parsing issues)
- Proper handle inheritance for stdin/stdout/stderr
- Clean process handle management

## Prerequisites

- **Node.js**: [Download](https://nodejs.org/) or install via package manager
- **Claude CLI**: Install via npm: `npm install -g @anthropic-ai/claude-code`
- **MinGW GCC**: [Download MinGW](https://sourceforge.net/projects/mingw/) or use a package manager

## Configuration (REQUIRED)

Before building, you **must** edit `claude_node_wrapper.c` to set your paths:

```c
#define NODE_EXE_PATH L"C:\\Program Files\\nodejs\\node.exe"
#define CLI_JS_PATH L"C:\\Users\\USERNAME\\AppData\\Roaming\\npm\\node_modules\\@anthropic-ai\\claude-code\\cli.js"
```

### Finding Your Paths

**Node.js path:**
```batch
where node
```

**CLI.js path:**
```batch
where claude
:: Look in the same directory for: node_modules\@anthropic-ai\claude-code\cli.js
```

### Common Path Configurations

**Standard Node.js + npm:**
```c
#define NODE_EXE_PATH L"C:\\Program Files\\nodejs\\node.exe"
#define CLI_JS_PATH L"C:\\Users\\YourName\\AppData\\Roaming\\npm\\node_modules\\@anthropic-ai\\claude-code\\cli.js"
```

**Scoop:**
```c
#define NODE_EXE_PATH L"C:\\Users\\YourName\\scoop\\apps\\nodejs-lts\\current\\node.exe"
#define CLI_JS_PATH L"C:\\Users\\YourName\\scoop\\persist\\nodejs-lts\\npm-global\\node_modules\\@anthropic-ai\\claude-code\\cli.js"
```

**nvm-windows:**
```c
#define NODE_EXE_PATH L"C:\\Users\\YourName\\AppData\\Roaming\\nvm\\v20.x.x\\node.exe"
#define CLI_JS_PATH L"C:\\Users\\YourName\\AppData\\Roaming\\npm\\node_modules\\@anthropic-ai\\claude-code\\cli.js"
```

## Building

### Quick Build

```batch
build.bat
```

### Manual Build

```batch
gcc -o claude-wrapper.exe claude_node_wrapper.c -municode -mconsole -O2 -s
```

### Debug Build

```batch
gcc -o claude-wrapper.exe claude_node_wrapper.c -municode -mconsole -DDEBUG
```

## Installation

Copy `claude-wrapper.exe` to a permanent location:

```batch
:: Option 1: Dedicated tools folder
mkdir C:\Tools
copy claude-wrapper.exe C:\Tools\

:: Option 2: npm global directory
copy claude-wrapper.exe %APPDATA%\npm\
```

## VS Code Configuration

Add to your VS Code `settings.json`:

```json
{
  "claudeCode.claudeProcessWrapper": "C:\\Tools\\claude-wrapper.exe"
}
```

**Important:** Use double backslashes (`\\`) in JSON paths.

**Settings file location:** `%APPDATA%\Code\User\settings.json`

## Testing

### Command Line Test

```batch
claude-wrapper.exe --version
:: Expected: 2.x.x (Claude Code)

claude-wrapper.exe --help
:: Expected: Usage information
```

### Run Test Suite

```batch
test.bat
```

## Compatibility

| Extension Version | Runtime | Without Wrapper | With Wrapper |
|-------------------|---------|-----------------|--------------|
| 2.0.28 | Node.js | Works | Works |
| 2.1.7+ | Bun (bundled) | CRASHES | Works |

The wrapper provides a **universal solution** that works with any extension version.

## Troubleshooting

### Error: Node.js not found

1. Run `where node` to find your Node.js path
2. Edit `claude_node_wrapper.c` and update `NODE_EXE_PATH`
3. Rebuild with `build.bat`

### Error: CLI script not found

1. Run `where claude` to find your Claude CLI location
2. Look for `node_modules\@anthropic-ai\claude-code\cli.js` in that directory
3. Edit `claude_node_wrapper.c` and update `CLI_JS_PATH`
4. Rebuild with `build.bat`

### Extension Still Crashes

1. Verify wrapper path in `settings.json` is correct
2. Ensure double backslashes in path
3. Restart VS Code completely after changing settings
4. Test wrapper from command line: `claude-wrapper.exe --version`

### GCC Not Found

- Download MinGW: https://sourceforge.net/projects/mingw/
- Or use Scoop: `scoop install gcc`
- Or use Chocolatey: `choco install mingw`

## Technical Details

The wrapper uses Windows API `CreateProcessW` to directly spawn `node.exe`:
- No `cmd.exe` intermediary (avoids argument parsing issues)
- Inherits stdin/stdout/stderr handles
- Preserves all argument quoting and special characters
- Returns child process exit code

## File Structure

```
claude-wrapper/
├── claude_node_wrapper.c   # Main source - EDIT PATHS HERE
├── build.bat               # Build script
├── test.bat                # Test script
├── README.md               # This file
├── INSTALL.md              # Detailed installation guide
├── QUICKSTART.md           # Quick reference
├── ARCHITECTURE.md         # Technical documentation
├── COMPARISON.md           # Comparison with original solution
├── .gitignore              # Git ignore rules
└── claude-wrapper.exe      # Compiled output (after build)
```

## Why This Works

The original GitHub solution using `cmd /c claude.cmd` failed because:
1. `cmd.exe` mangles complex JSON arguments
2. Special characters get interpreted by the shell
3. Long system prompts with markdown break parsing

Our solution bypasses `cmd.exe` entirely by calling `node.exe` directly with the CLI script path, preserving all argument integrity.

## Credits

- Original concept from GitHub issue anthropics/claude-code#11201
- Final working implementation developed through iterative debugging

## License

MIT License - See LICENSE file for details.

## Version History

- **v1.0** - Initial `cmd /c claude.cmd` wrapper (failed with complex args)
- **v2.0** - Transparent pass-through wrapper (didn't bypass Bun)
- **v3.0** - **Direct Node.js wrapper (WORKING!)** - Current version
