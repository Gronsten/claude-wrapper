# Installation Guide

Complete installation guide for the Claude CLI Windows wrapper.

## Prerequisites

### 1. Node.js (Required)

Check if Node.js is installed:
```batch
node --version
```

If not installed:
```batch
scoop install nodejs-lts
```

### 2. Claude CLI (Required)

Check if Claude CLI is installed:
```batch
claude --version
```

If not installed:
```batch
npm install -g @anthropic-ai/claude-code
```

### 3. GCC Compiler (Required for building)

Check if GCC is available:
```batch
gcc --version
```

If not installed:
```batch
scoop install gcc
```

## Build Instructions

### Step 1: Navigate to Project Directory

```batch
cd path\to\claude-wrapper
```

### Step 2: Build the Wrapper

```batch
build.bat
```

Expected output:
```
========================================
Building Claude Wrapper for Windows
========================================

[1/3] Checking gcc version...
gcc (x86_64-posix-seh-rev0, Built by MinGW-Builds project) 15.2.0

[2/3] Compiling claude_node_wrapper.c...
      Successfully compiled: claude-wrapper.exe

[3/3] Verifying output...
      Output file exists: claude-wrapper.exe
      File size: 17408 bytes

========================================
Build completed successfully!
========================================
```

### Step 3: Test the Wrapper

```batch
test.bat
```

All 5 tests should pass:
- Version check
- Help text
- Simple calculation
- Multiple arguments
- Exit code forwarding

## Installation Options

### Option A: Dedicated Tools Directory (Recommended)

```batch
mkdir C:\Tools
copy claude-wrapper.exe C:\Tools\
```

**VS Code Configuration:**
```json
{
  "claudeCode.claudeProcessWrapper": "C:\\Tools\\claude-wrapper.exe"
}
```

### Option B: npm Global Directory

```batch
copy claude-wrapper.exe %APPDATA%\npm\
```

**VS Code Configuration:**
```json
{
  "claudeCode.claudeProcessWrapper": "C:\\Users\\YourName\\AppData\\Roaming\\npm\\claude-wrapper.exe"
}
```

## Configure VS Code

### Step 1: Locate Your Settings File

**Standard VS Code:**
```
%APPDATA%\Code\User\settings.json
```

**Scoop VS Code:**
```
%USERPROFILE%\scoop\persist\vscode\data\user-data\User\settings.json
```

### Step 2: Add Wrapper Configuration

Add this line to your `settings.json`:

```json
{
  "claudeCode.claudeProcessWrapper": "C:\\Tools\\claude-wrapper.exe"
}
```

**Important:** Use double backslashes (`\\`) in JSON paths.

### Step 3: Restart VS Code

Close and reopen VS Code completely for the changes to take effect.

## Verification

### Test in Command Line

```batch
C:\Tools\claude-wrapper.exe --version
```

Expected output:
```
2.1.7 (Claude Code)
```

### Test in VS Code

1. Open VS Code
2. Open the Claude Code panel
3. Start a new session
4. If no errors appear, the wrapper is working!

### Check VS Code Logs

If issues occur, check the Claude Code extension logs:
1. Open Command Palette (`Ctrl+Shift+P`)
2. Type "Developer: Open Extension Logs Folder"
3. Look for `Anthropic.claude-code` folder
4. Open `Claude VSCode.log`

## Troubleshooting

### Error: Node.js not found

**Cause:** The hardcoded path in `claude_node_wrapper.c` doesn't match your Node.js installation.

**Solution:**
1. Find your Node.js path:
   ```batch
   where node
   ```
2. Edit `claude_node_wrapper.c` line 18:
   ```c
   #define NODE_EXE_PATH L"C:\\Your\\Path\\To\\node.exe"
   ```
3. Rebuild:
   ```batch
   build.bat
   ```

### Error: CLI script not found

**Cause:** The hardcoded CLI path doesn't match your npm installation.

**Solution:**
1. Find your claude.cmd location:
   ```batch
   where claude.cmd
   ```
2. The `cli.js` is in the same parent directory under:
   ```
   node_modules\@anthropic-ai\claude-code\cli.js
   ```
3. Edit `claude_node_wrapper.c` line 19 and rebuild.

### Error: GCC not found

**Solution:**
```batch
scoop install gcc
```

### Error: VS Code doesn't recognize the wrapper

**Checks:**
1. Verify path in `settings.json` is correct and absolute
2. Ensure double backslashes in JSON path
3. Check that `claude-wrapper.exe` exists at the specified location
4. Restart VS Code completely after changing settings

### Error: "Access denied" when launching

**Solutions:**
1. Check file permissions on `claude-wrapper.exe`
2. Add exception to antivirus for `claude-wrapper.exe`
3. Ensure the wrapper isn't locked by another process

### Extension Still Shows Bun Crash

**Checks:**
1. Verify `claudeCode.claudeProcessWrapper` setting is not commented out
2. Ensure the path uses double backslashes
3. Confirm wrapper works from command line first
4. Try completely closing VS Code (all windows) and reopening

## Customizing Paths

If your Node.js or npm installation is in a different location, edit `claude_node_wrapper.c`:

```c
// Line 18: Path to node.exe
#define NODE_EXE_PATH L"C:\\Your\\Path\\To\\node.exe"

// Line 19: Path to cli.js
#define CLI_JS_PATH L"C:\\Your\\Path\\To\\node_modules\\@anthropic-ai\\claude-code\\cli.js"
```

Then rebuild with `build.bat`.

## Updating

### When Claude CLI Updates

The wrapper will continue to work as long as:
1. Node.js path remains the same
2. The `cli.js` location remains the same

If paths change after an update, edit the paths in `claude_node_wrapper.c` and rebuild.

### When Node.js Updates

If Node.js is updated via Scoop, the path typically remains the same (`current` symlink). No action needed.

## Uninstallation

To remove the wrapper:

1. Delete `claude-wrapper.exe` from installation location
2. Remove `claudeCode.claudeProcessWrapper` setting from VS Code `settings.json`
3. Restart VS Code

The Claude Code extension will revert to using its bundled executable.

## Support

For issues with:
- **This wrapper:** Check troubleshooting section above
- **Claude CLI:** https://github.com/anthropics/claude-code
- **VS Code extension:** Check extension marketplace page
- **Compilation errors:** Verify GCC installation

## How It Works

The wrapper bypasses the buggy Bun executable bundled with the VS Code extension by:

1. Receiving all command-line arguments from the extension
2. Calling `node.exe` directly with the npm-installed `cli.js`
3. Passing all arguments unchanged (preserving JSON and special characters)
4. Returning the exit code to the extension

This allows the extension to function normally while using the stable Node.js runtime instead of Bun.
