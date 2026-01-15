# Quick Start Guide

Get up and running in 5 minutes.

## TL;DR

```batch
# 1. Edit claude_node_wrapper.c with YOUR paths (see below)

# 2. Build
build.bat

# 3. Test
test.bat

# 4. Install
copy claude-wrapper.exe C:\Tools\

# 5. Configure VS Code settings.json
{
  "claudeCode.claudeProcessWrapper": "C:\\Tools\\claude-wrapper.exe"
}

# 6. Restart VS Code
```

## REQUIRED: Configure Your Paths

Edit `claude_node_wrapper.c` before building:

```c
// Find your paths with: where node
#define NODE_EXE_PATH L"C:\\Program Files\\nodejs\\node.exe"

// Find your paths with: where claude (then look for cli.js)
#define CLI_JS_PATH L"C:\\Users\\YourName\\AppData\\Roaming\\npm\\node_modules\\@anthropic-ai\\claude-code\\cli.js"
```

## What This Does

Bypasses the crashing Bun executable in Claude Code extension 2.1.7+ by routing calls to the stable Node.js CLI.

**Before:** Extension → Bundled Bun → CRASH
**After:** Extension → Wrapper → Node.js → SUCCESS

## Commands Reference

| Command | Purpose |
|---------|---------|
| `build.bat` | Compile the wrapper |
| `test.bat` | Run test suite |
| `claude-wrapper.exe --version` | Verify wrapper works |
| `claude-wrapper.exe --help` | Show Claude CLI help |

## File Reference

| File | Purpose |
|------|---------|
| `claude_node_wrapper.c` | Main source code - **EDIT PATHS HERE** |
| `claude-wrapper.exe` | Compiled wrapper (Git-ignored) |
| `build.bat` | Build script |
| `test.bat` | Test script |
| `README.md` | Full documentation |
| `INSTALL.md` | Detailed installation guide |
| `.gitignore` | Git ignore rules |

## Common Path Configurations

**Standard npm:**
```
Node: C:\Program Files\nodejs\node.exe
CLI:  C:\Users\<user>\AppData\Roaming\npm\node_modules\@anthropic-ai\claude-code\cli.js
```

**Scoop:**
```
Node: C:\Users\<user>\scoop\apps\nodejs-lts\current\node.exe
CLI:  C:\Users\<user>\scoop\persist\nodejs-lts\npm-global\node_modules\@anthropic-ai\claude-code\cli.js
```

## VS Code Settings Format

**Always use double backslashes in JSON:**

✅ Correct:
```json
"claudeCode.claudeProcessWrapper": "C:\\Tools\\claude-wrapper.exe"
```

❌ Wrong:
```json
"claudeCode.claudeProcessWrapper": "C:\Tools\claude-wrapper.exe"
```

## Troubleshooting Quick Fixes

| Problem | Quick Fix |
|---------|-----------|
| GCC not found | Install MinGW or `scoop install gcc` |
| Test fails | Check paths in `claude_node_wrapper.c` |
| VS Code still crashes | Restart VS Code completely |
| Path not found | Run `where node` and `where claude` to find correct paths |

## Compatibility

| Extension | Status |
|-----------|--------|
| 2.0.28 (Node.js) | ✅ Works |
| 2.1.7+ (Bun) | ✅ Works (bypasses Bun) |

## Need More Help?

- [README.md](README.md) - Full documentation
- [INSTALL.md](INSTALL.md) - Step-by-step installation
- Check VS Code logs for detailed error messages
