# Quick Start Guide

Get up and running in 5 minutes.

## TL;DR (Pre-built Release)

```batch
# 1. Download claude-wrapper.exe from Releases

# 2. Copy to permanent location
copy claude-wrapper.exe C:\Tools\

# 3. Create claude-wrapper.json in same directory
{
  "nodePath": "C:\\Program Files\\nodejs\\node.exe",
  "cliPath": "C:\\Users\\YourName\\AppData\\Roaming\\npm\\node_modules\\@anthropic-ai\\claude-code\\cli.js"
}

# 4. Configure VS Code settings.json
{
  "claudeCode.claudeProcessWrapper": "C:\\Tools\\claude-wrapper.exe"
}

# 5. Restart VS Code
```

## TL;DR (Build from Source)

```batch
# 1. Create claude-wrapper.json with your paths

# 2. Build
build.bat

# 3. Test
test.bat

# 4. Install
copy claude-wrapper.exe C:\Tools\
copy claude-wrapper.json C:\Tools\

# 5. Configure VS Code settings.json
{
  "claudeCode.claudeProcessWrapper": "C:\\Tools\\claude-wrapper.exe"
}

# 6. Restart VS Code
```

## REQUIRED: Configuration File

Create `claude-wrapper.json` in the same directory as the .exe:

```json
{
  "nodePath": "C:\\path\\to\\node.exe",
  "cliPath": "C:\\path\\to\\cli.js"
}
```

Find your paths:
```batch
where node        # -> nodePath
where claude      # -> Look for cli.js in same directory
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
| `claude_node_wrapper.c` | Main source code |
| `claude-wrapper.json` | Your configuration (required) |
| `claude-wrapper.example.json` | Example configuration |
| `claude-wrapper.exe` | Compiled wrapper (Git-ignored) |
| `build.bat` | Build script |
| `test.bat` | Test script |

## Common Path Configurations

**Standard npm:**
```json
{
  "nodePath": "C:\\Program Files\\nodejs\\node.exe",
  "cliPath": "C:\\Users\\<user>\\AppData\\Roaming\\npm\\node_modules\\@anthropic-ai\\claude-code\\cli.js"
}
```

**Scoop:**
```json
{
  "nodePath": "C:\\Users\\<user>\\scoop\\apps\\nodejs-lts\\current\\node.exe",
  "cliPath": "C:\\Users\\<user>\\scoop\\persist\\nodejs-lts\\npm-global\\node_modules\\@anthropic-ai\\claude-code\\cli.js"
}
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
| Config not found | Ensure `claude-wrapper.json` is in same directory as .exe |
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
