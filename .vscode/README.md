# VS Code Build Automation

This directory contains VS Code task configurations for N64 ROM development.

## What's Here

- **tasks.json** - Build tasks and utilities for N64 development

**Note:**
- `launch.json` is **not used** - Can't attach debugger to N64 emulators from VS Code
- `settings.json` is **managed in devcontainer.json** - All extension/workspace settings are in `.devcontainer/devcontainer.json` for consistency

---

## Quick Reference

### Primary Workflow

| Action | Keyboard Shortcut |
|--------|-------------------|
| **Quick Build** | `Cmd+Shift+B` → `Enter` |
| **Open Build Menu** | `Cmd+Shift+B` |
| **Run Any Task** | `Cmd+Shift+P` → "Run Task" |

---

## Available Tasks

### 🎮 Primary Build Tasks

**Build ROM** (Default - `Cmd+Shift+B` → `Enter`)
- Quick incremental build
- Optimized for performance (`-O2`)
- Only recompiles changed files
- **Use for:** Regular development iteration

**Build ROM (Debug)**
- Incremental build with debug symbols
- No optimization (`-O0`), includes `-g` flag
- Larger ROM, easier to debug
- **Use for:** When you need debug symbols for emulator debugging tools

### 🔨 Clean Build Tasks

**Rebuild All**
- Clean build directory + full rebuild
- Optimized build (`-O2`)
- **Use for:** When incremental builds act weird, or before release

**Rebuild All (Debug)**
- Clean + debug build
- **Use for:** Full clean debug build

### 🧰 Utility Tasks

**Clean Build Artifacts**
- Removes `build/` directory and `.z64` ROM
- **Use for:** Starting fresh, freeing disk space

**Show ROM Info**
- Displays ROM file size and type information
- **Use for:** Checking ROM size, verifying it exists

**Open ROM Location**
- Shows instructions for opening ROM in Ares
- **Use for:** Quick reminder of where the ROM is on your Mac

---

## Understanding Build Modes

| Mode | Flags | ROM Size | Performance | Debug Info | Use Case |
|------|-------|----------|-------------|------------|----------|
| **Release** | `-O2` | Smaller | Faster | ❌ No | Regular development, distribution |
| **Debug** | `-g -O0` | Larger | Slower | ✅ Yes | Debugging with emulator tools |

**Release Mode:**
- Code is optimized (functions may be inlined, loops unrolled)
- Faster execution on N64
- Harder to match source code to assembly when debugging
- **Default for most development**

**Debug Mode:**
- No optimizations - code structure matches source closely
- Includes debug symbols
- Easier to use with GDB or emulator debugging features
- Slower execution but more debuggable

---

## Workflow Examples

### Quick Iteration (Most Common)
```
1. Edit code in src/main.c
2. Cmd+Shift+B → Enter (quick build)
3. Double-click n64-tech-demo.z64 in Finder to launch Ares
4. Test changes
5. Repeat
```

### Clean Build Before Release
```
1. Cmd+Shift+P → "Run Task" → "Rebuild All"
2. Verify ROM works in Ares
3. Distribute n64-tech-demo.z64
```

### Debugging Session
```
1. Cmd+Shift+P → "Run Task" → "Build ROM (Debug)"
2. Open ROM in Ares
3. Use Ares debugger features with debug symbols
```

### Check ROM Size
```
1. Cmd+Shift+P → "Run Task" → "Show ROM Info"
2. See file size and type information
```

---

## File Organization

```
.vscode/
├── tasks.json      # Build task definitions
└── README.md       # This file
```

**Configuration:**
- **tasks.json** - Build tasks organized into sections:
  - Primary Build Tasks - Your main build commands
  - Clean Build Tasks - Full rebuilds from scratch
  - Utility Tasks - Information and helpers

- **Extension Settings** - Located in `.devcontainer/devcontainer.json`:
  - CMake disabled (project uses Make, not CMake)
  - C/C++ IntelliSense configured for MIPS development
  - File exclusions and associations
  - Terminal defaults

---

## Customization

You can add your own tasks to `tasks.json`:

```json
{
  "label": "Your Custom Task",
  "type": "shell",
  "command": "your-command-here",
  "group": "build",
  "detail": "Description shown in task picker"
}
```

Common customizations:
- Asset compilation tasks
- ROM analysis tools
- Automated testing
- Deployment scripts

See [VS Code Tasks Documentation](https://code.visualstudio.com/docs/editor/tasks) for more information.

---

## Why No launch.json?

**launch.json** is for running applications with an **attached debugger** (like debugging a local C program with GDB).

For N64 development:
- ❌ VS Code can't attach to N64 emulators
- ❌ Emulators run on host Mac (tasks run in Linux container)
- ❌ N64 debugging requires specialized emulator features, not VS Code

Instead:
- ✅ Use **tasks.json** for building
- ✅ Use **Ares emulator** on your Mac for running/debugging
- ✅ Use **emulator's built-in debugger** if you need to debug assembly/hardware

This is the **standard workflow** for embedded/console development in VS Code.
