# Copilot Instructions: N64 Tech Demo

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
- **Clean build**: `make clean && make`
- **VS Code integration**: Use `Cmd+Shift+B` for build tasks or `F5` to build & launch

### Testing & Debugging
- ROMs output as `n64-tech-demo.z64` file
- Test with Ares emulator (recommended) or other N64 emulators
- Debug builds include symbols for emulator debugging support
- VS Code launch configurations auto-build and launch ROMs in Ares

## Critical Project Conventions

### File Structure
```
src/           # All C source code
build/         # Generated build artifacts (gitignored)  
.devcontainer/ # Dev Container configuration (Docker)
.vscode/       # VS Code tasks and launch configs
```

### Build Configuration (Makefile)
- **Project name**: Controls output ROM filename via `PROJECT_NAME` variable
- **ROM title**: Set via `N64_ROM_TITLE` (appears in emulator)
- **Source files**: Listed in `SRC` variable, auto-compiled to `$(BUILD_DIR)/%.o`
- **Libdragon integration**: Uses `include $(N64_INST)/include/n64.mk` for toolchain

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
- **Compilation**: C11 standard (`-std=c11`)
- **Linking**: Handled automatically by Libdragon's n64.mk
- **ROM size**: 1MB default (`n64tool -l 1M`)

## Integration Points

### VS Code Integration
- **Tasks**: Defined in `.vscode/tasks.json` with GCC problem matcher
- **Launch configs**: Platform-specific emulator launching (macOS/Linux/Windows)
- **C/C++ extension**: Pre-configured for MIPS toolchain intellisense

### External Dependencies
- **Libdragon Docker image**: `ghcr.io/dragonminded/libdragon:latest`
- **Emulator requirement**: Ares emulator on host machine for F5 workflow
- **Cross-platform compatibility**: All builds produce identical MIPS binaries

## Common Development Patterns

When adding new source files:
1. Add to `SRC` variable in Makefile
2. Follow Libdragon initialization patterns
3. Use manual rendering mode for performance control
4. Test with both optimized and debug builds

When debugging:
- Use `make debug` for symbol information
- Emulator debugging requires debug ROM build
- Console output appears in emulator, not VS Code terminal

When modifying build:
- ROM title changes require Makefile modification
- Additional libraries need Libdragon makefile integration
- Clean builds recommended after Makefile changes