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
1. Loads configuration from `claude-wrapper.json` (same directory as .exe)
2. Receives all arguments from the VS Code extension
3. Calls `node.exe` directly with the `cli.js` script path
4. Passes all arguments unchanged (preserving JSON, special characters, etc.)
5. Returns the exit code to the extension

## Security Features

- Buffer overflow protection (32KB command line limit check)
- Direct process creation (bypasses cmd.exe parsing issues)
- Proper handle inheritance for stdin/stdout/stderr
- Clean process handle management

## Prerequisites

- **Node.js**: [Download](https://nodejs.org/) or install via package manager
- **Claude CLI**: Install via npm: `npm install -g @anthropic-ai/claude-code`
- **MinGW GCC**: Only needed if building from source

## Quick Start (Pre-built Release)

1. Download `claude-wrapper.exe` from [Releases](https://github.com/Gronsten/claude-wrapper/releases)
2. Copy to a permanent location (e.g., `C:\Tools\`)
3. Create `claude-wrapper.json` in the same directory:

```json
{
  "nodePath": "C:\\Program Files\\nodejs\\node.exe",
  "cliPath": "C:\\Users\\YourName\\AppData\\Roaming\\npm\\node_modules\\@anthropic-ai\\claude-code\\cli.js"
}
```

4. Configure VS Code `settings.json`:

```json
{
  "claudeCode.claudeProcessWrapper": "C:\\Tools\\claude-wrapper.exe"
}
```

5. Restart VS Code

## Configuration

Create `claude-wrapper.json` in the same directory as `claude-wrapper.exe`:

```json
{
  "nodePath": "C:\\path\\to\\node.exe",
  "cliPath": "C:\\path\\to\\cli.js"
}
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
```json
{
  "nodePath": "C:\\Program Files\\nodejs\\node.exe",
  "cliPath": "C:\\Users\\YourName\\AppData\\Roaming\\npm\\node_modules\\@anthropic-ai\\claude-code\\cli.js"
}
```

**Scoop:**
```json
{
  "nodePath": "C:\\Users\\YourName\\scoop\\apps\\nodejs-lts\\current\\node.exe",
  "cliPath": "C:\\Users\\YourName\\scoop\\persist\\nodejs-lts\\npm-global\\node_modules\\@anthropic-ai\\claude-code\\cli.js"
}
```

**nvm-windows:**
```json
{
  "nodePath": "C:\\Users\\YourName\\AppData\\Roaming\\nvm\\v20.x.x\\node.exe",
  "cliPath": "C:\\Users\\YourName\\AppData\\Roaming\\npm\\node_modules\\@anthropic-ai\\claude-code\\cli.js"
}
```

## Building from Source

### Quick Build

```batch
build.bat
```

### Manual Build

```batch
gcc -o claude-wrapper.exe claude_node_wrapper.c -municode -mconsole -O2 -s -lshlwapi
```

### Debug Build

```batch
gcc -o claude-wrapper.exe claude_node_wrapper.c -municode -mconsole -DDEBUG -lshlwapi
```

## Installation

1. Copy `claude-wrapper.exe` to a permanent location:

```batch
:: Option 1: Dedicated tools folder
mkdir C:\Tools
copy claude-wrapper.exe C:\Tools\

:: Option 2: npm global directory
copy claude-wrapper.exe %APPDATA%\npm\
```

2. Create `claude-wrapper.json` in the same directory with your paths

3. Configure VS Code `settings.json`:

```json
{
  "claudeCode.claudeProcessWrapper": "C:\\Tools\\claude-wrapper.exe"
}
```

**Important:** Use double backslashes (`\\`) in JSON paths.

**Settings file location:** `%APPDATA%\Code\User\settings.json`

## CLI Usage

### Wrapper-Specific Commands

The wrapper provides its own CLI commands that are handled before passthrough:

```batch
claude-wrapper.exe --wrapper-version
:: Show wrapper version (e.g., "claude-wrapper version 1.2.0")

claude-wrapper.exe --wrapper-help
:: Show wrapper-specific help and usage

claude-wrapper.exe --wrapper-test
:: Test Claude Code passthrough (verifies config, Node.js, CLI, and passthrough)
```

**Note:** These wrapper commands start with `--wrapper-` prefix to avoid conflicts with Claude Code CLI flags.

### Testing

#### Quick Test

```batch
claude-wrapper.exe --wrapper-test
```

This runs a comprehensive test that checks:
1. Configuration file loading
2. Node.js existence
3. Claude Code CLI existence
4. Actual passthrough execution

#### Manual Tests

```batch
claude-wrapper.exe --version
:: Expected: 2.x.x (Claude Code) - passes through to Claude Code CLI

claude-wrapper.exe --help
:: Expected: Claude Code CLI help - passes through to Claude Code CLI

claude-wrapper.exe --wrapper-version
:: Expected: claude-wrapper version 1.2.0 - handled by wrapper
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
2. Update `nodePath` in `claude-wrapper.json`

### Error: CLI script not found

1. Run `where claude` to find your Claude CLI location
2. Look for `node_modules\@anthropic-ai\claude-code\cli.js` in that directory
3. Update `cliPath` in `claude-wrapper.json`

### Extension Still Crashes

1. Verify wrapper path in `settings.json` is correct
2. Ensure double backslashes in path
3. Verify `claude-wrapper.json` is in same directory as `.exe`
4. Restart VS Code completely after changing settings
5. Test wrapper from command line: `claude-wrapper.exe --version`

### GCC Not Found (Building from Source)

- Download MinGW: https://sourceforge.net/projects/mingw/
- Or use Scoop: `scoop install gcc`
- Or use Chocolatey: `choco install mingw`

## Technical Details

The wrapper uses Windows API `CreateProcessW` to directly spawn `node.exe`:
- No `cmd.exe` intermediary (avoids argument parsing issues)
- Inherits stdin/stdout/stderr handles
- Preserves all argument quoting and special characters
- Returns child process exit code
- Reads configuration from JSON file (no recompilation needed)

## File Structure

```
claude-wrapper/
├── claude_node_wrapper.c       # Main source code
├── claude-wrapper.example.json # Example configuration
├── build.bat                   # Build script
├── test.bat                    # Test script
├── README.md                   # This file
├── INSTALL.md                  # Detailed installation guide
├── QUICKSTART.md               # Quick reference
├── ARCHITECTURE.md             # Technical documentation
├── COMPARISON.md               # Comparison with original solution
├── LICENSE                     # MIT License
├── .gitignore                  # Git ignore rules
└── claude-wrapper.exe          # Compiled output (after build)
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

- **v1.2.0** - Added wrapper-specific CLI switches (--wrapper-version, --wrapper-help, --wrapper-test)
- **v1.1.0** - Added JSON config file support (no recompilation needed)
- **v1.0.0** - Initial release with hardcoded paths
