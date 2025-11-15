# GitHub Copilot Instructions: N64 Tech Demo

## Critical Requirements

### Commit and Pull Request Policy
**NEVER include ANY of the following in commit messages, PR descriptions, or git history:**
- References to AI coding assistants (Claude, Claude Code, Anthropic, GitHub Copilot, etc.)
- Emojis of any kind
- Co-authored-by lines for AI tools
- Attribution to automated tools

All git history must appear as if written entirely by humans. Keep commit messages professional, concise, and focused on what changed and why.

### Code Comments and Documentation
Same policy applies to all code comments and documentation:
- No emoji usage
- No references to AI tools
- Write as if all code is human-authored

## Project Architecture

This is a Nintendo 64 homebrew project built with **Libdragon** - a modern N64 development SDK. The project uses Dev Containers to provide cross-platform development (macOS, Linux, Windows) with a consistent MIPS toolchain.

### Key Components
- **Dev Container**: Cross-platform development environment using `linux/amd64` platform for Rosetta 2 compatibility on Apple Silicon
- **Libdragon SDK**: Modern N64 development framework (replaces traditional libultra)
- **MIPS Cross-Compiler**: `mips64-elf-gcc` toolchain for N64's VR4300 processor
- **Build System**: GNU Make with Libdragon's `n64.mk` integration

## Essential Development Workflow

### Building ROMs
- **Primary build**: `make` (optimized `-O2` build)
- **Debug build**: `make debug` (adds `-g -O0` flags)
- **Clean rebuild**: `make clean && make`
- **VS Code integration**: Use `Cmd+Shift+B` for build tasks menu

### Testing & Debugging
- ROMs output as `n64-tech-demo.z64` file in project root
- Build in VS Code Dev Container, then double-click ROM in Finder to launch Ares
- Test with Ares emulator (recommended) or other N64 emulators
- Debug builds include symbols for emulator debugging tools

## Critical Project Conventions

### File Structure
```
src/           # All C source code
build/         # Generated build artifacts (gitignored)
.devcontainer/ # Dev Container configuration (Docker)
.vscode/       # VS Code build tasks
```

### Build Configuration (Makefile)
- **Project name**: Controls output ROM filename via `PROJECT_NAME` variable
- **ROM title**: Set via `N64_ROM_TITLE` (appears in emulator)
- **Source files**: Listed in `SRCS` variable (relative to SOURCE_DIR), auto-compiled to `$(BUILD_DIR)/%.o`
- **Compiler standard**: Uses `-std=gnu11` (GNU extensions required by Libdragon)
- **Libdragon integration**: Uses `include $(N64_INST)/include/n64.mk` for build system

### Libdragon Patterns
- **Initialization sequence**: `console_init()` → `console_set_render_mode()` → `joypad_init()`
- **Main loop**: Poll joypad → process input → render → repeat
- **Manual rendering**: Use `RENDER_MANUAL` mode with explicit `console_render()` calls
- **Controller input**: `joypad_poll()` → `joypad_get_inputs(JOYPAD_PORT_1)`

## Platform Considerations

### Dev Container Requirements
- **Apple Silicon**: Requires `platform: linux/amd64` in docker-compose for Rosetta 2 emulation
- **WSL2**: Repository must be cloned inside WSL2 filesystem, not Windows mount
- **Toolchain location**: `/n64_toolchain/bin/mips64-elf-gcc` (configured in VS Code C++ settings)

### Build Output Details
- **ROM format**: `.z64` (big-endian N64 ROM format)
- **Compilation**: GNU C11 standard (`-std=gnu11` - required for Libdragon)
- **Optimization**: `-O2` for release builds, `-O0` for debug builds
- **Linking**: Handled automatically by Libdragon's n64.mk build system

## Integration Points

### VS Code Integration
- **Tasks**: Defined in `.vscode/tasks.json` with GCC problem matcher and multiple build options
- **C/C++ extension**: Pre-configured for MIPS toolchain IntelliSense
- **Settings**: Consolidated in `.devcontainer/devcontainer.json` for consistency

### External Dependencies
- **Libdragon Docker image**: `ghcr.io/dragonminded/libdragon:latest` (base toolchain)
- **Libdragon SDK**: Preview branch installed during container build
- **Emulator requirement**: Ares emulator on host machine for testing
- **Cross-platform compatibility**: All builds produce identical MIPS binaries

## Common Development Patterns

When adding new source files:
1. Add filename to `SRCS` variable in Makefile (relative to src/ directory)
2. Follow Libdragon initialization patterns (see API documentation)
3. Test with both optimized and debug builds
4. Use `make clean` before full rebuild if needed

When debugging:
- Use `make debug` for debug symbols and no optimization
- Emulator debugging requires debug ROM build
- Console output appears in emulator, not VS Code terminal
- Use emulator's built-in debugging tools for assembly/hardware debugging

When modifying build:
- ROM title changes require Makefile modification
- Additional compiler flags go in `N64_CFLAGS +=` lines
- Clean builds recommended after Makefile changes
- All source files must be in `src/` directory