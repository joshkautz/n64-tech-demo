# N64 Tech Demo - Development Instructions

This is a Nintendo 64 homebrew project built with the Libdragon SDK. These instructions guide development practices, conventions, and technical requirements.

## Critical Requirements

### Commit and PR Message Policy
**NEVER include ANY of the following in commit messages, PR descriptions, or git history:**
- References to AI coding assistants (Claude, Claude Code, Anthropic, GitHub Copilot, etc.)
- Emojis of any kind
- Co-authored-by lines for AI tools
- Attribution to automated tools

All git history must appear as if written entirely by humans. Keep commit messages professional, concise, and focused on what changed and why.

**Good commit message:**
```
Add sprite rendering system

Implements basic sprite batch rendering using RDP display lists for efficient texture drawing.
```

**Bad commit message:**
```
✨ Add sprite rendering system 🎮

Generated with Claude Code
Co-Authored-By: Claude <noreply@anthropic.com>
```

### Code Comments and Documentation
Same policy applies to all code comments and documentation:
- No emoji usage
- No references to AI tools
- Write as if all code is human-authored

## Project Architecture

### Build Environment
- **Dev Container**: All development happens in a containerized Linux environment
- **Platform**: Docker with linux/amd64 (runs via Rosetta 2 on Apple Silicon)
- **Toolchain**: mips64-elf-gcc cross-compiler targeting MIPS R4300i
- **SDK**: Libdragon preview branch (includes OpenGL 1.1, MPEG1 video, TrueType fonts, multithreading)
- **Build System**: GNU Make with libdragon's n64.mk build system

### Project Structure
```
n64-tech-demo/
├── .devcontainer/          # Dev Container configuration
│   ├── devcontainer.json   # VS Code settings and extensions
│   ├── docker-compose.yml  # Container orchestration
│   └── Dockerfile          # Custom image with Libdragon SDK
├── .vscode/                # VS Code tasks and documentation
│   ├── tasks.json          # Build tasks (Cmd+Shift+B)
│   └── README.md           # Build system documentation
├── src/                    # C source files
│   └── main.c              # Entry point
├── build/                  # Build artifacts (gitignored)
├── Makefile                # Build configuration
└── *.z64                   # Output ROM file (gitignored)
```

## Build System Conventions

### Makefile Patterns
Libdragon uses a specialized build system defined in `$(N64_INST)/include/n64.mk`. Follow these conventions:

**Source files:**
```makefile
# Use SRCS for source files (relative to SOURCE_DIR)
SRCS = main.c graphics.c audio.c

# NOT this:
# SRC = src/main.c src/graphics.c  # Wrong!
```

**Compiler flags:**
```makefile
# Always use gnu11 (not c11) - Libdragon requires GNU extensions
N64_CFLAGS += -std=gnu11

# Build modes
ifndef DEBUG
  N64_CFLAGS += -O2          # Release: optimized
else
  N64_CFLAGS += -g -O0       # Debug: symbols, no optimization
endif
```

**ROM configuration:**
```makefile
# Set ROM title before building
$(PROJECT_NAME).z64: N64_ROM_TITLE="Your Title"
$(PROJECT_NAME).z64: $(BUILD_DIR)/$(PROJECT_NAME).elf
```

### Build Targets
- `make` - Incremental optimized build (-O2)
- `make debug` - Incremental debug build (-g -O0)
- `make clean` - Remove all build artifacts

**VS Code Integration:**
- `Cmd+Shift+B` → Enter - Quick build (default)
- `Cmd+Shift+P` → "Run Task" - Access all build tasks

## Development Workflow

### Two-Step Build and Test Cycle
1. **Build in VS Code** (inside Dev Container)
   - Edit code in `src/`
   - `Cmd+Shift+B` to build
   - ROM outputs to project root as `n64-tech-demo.z64`

2. **Test on Host Machine** (outside Dev Container)
   - Double-click `n64-tech-demo.z64` in Finder
   - ROM opens in Ares emulator
   - Test changes, iterate

**Why this workflow?**
- Build tools run in Linux container (proper toolchain environment)
- Emulator runs on macOS host (better performance, GPU access)
- This is standard for embedded/console development

### Dev Container Limitations
- Cannot run host commands from container tasks/launches
- No F5 debugging (N64 emulators don't support VS Code debugging)
- Use emulator's built-in debugger for assembly/hardware debugging

## Libdragon SDK Patterns

### Initialization Order
Most Libdragon programs follow this initialization pattern:

```c
int main(void)
{
    // 1. Initialize display
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE, FILTERS_RESAMPLE);

    // 2. Initialize subsystems you need
    rdp_init();           // Graphics hardware
    controller_init();    // Controller input
    timer_init();         // Timing system
    audio_init(44100, 4); // Audio (freq, buffers)

    // 3. Initialize your systems
    graphics_init();
    audio_init_music();

    // 4. Main game loop
    while(1)
    {
        // Game logic here
    }

    return 0;
}
```

### Memory Management
- N64 has 4MB RAM (8MB on expansion pak)
- Use static allocation where possible
- Be mindful of texture/audio data sizes
- ROM size limit: 64MB (practical limit ~32MB for cartridge emulation)

### Common Gotchas
1. **GNU Extensions Required**: Always use `-std=gnu11`, not `-std=c11`
2. **No Standard Library**: Limited libc - use Libdragon's implementations
3. **Display Buffering**: Use double/triple buffering - never write directly to active display
4. **RDP Pipeline**: Graphics commands must be in correct order (attach -> set modes -> draw -> detach)
5. **Audio Timing**: Audio callbacks run in interrupt context - keep them fast

## Graphics Development

### Display Management
```c
// Get framebuffer for current frame
display_context_t disp = display_get();

// Do all drawing operations
// ...

// Show completed frame
display_show(disp);
```

### RDP Usage
The RDP (Reality Display Processor) handles all graphics rendering:

```c
// Attach RDP to current display
rdp_attach_display(disp);

// Enable features
rdp_enable_texture_copy();

// Draw operations
rdp_draw_sprite(x, y, sprite);

// Detach when done
rdp_detach_display();
```

### Sprite Loading
```c
// Load sprite from filesystem
sprite_t *sprite = sprite_load("rom:/gfx/player.sprite");

// Use sprite...

// Free when done
sprite_free(sprite);
```

## Audio Development

### Audio System
Libdragon provides mixer-based audio:

```c
// Initialize audio (frequency, buffer count)
audio_init(44100, 4);

// Load WAV file
wav64_t sfx;
wav64_open(&sfx, "rom:/sfx/jump.wav64");

// Play sound
wav64_play(&sfx, 31); // channel 31 = SFX

// Close when done
wav64_close(&sfx);
```

### Music Playback
For background music, use XM (module) format:

```c
xm64player_t music;
xm64player_open(&music, "rom:/music/theme.xm64");
xm64player_play(&music, 0); // channel 0 = music
```

## Input Handling

### Controller Reading
```c
controller_scan();

struct controller_data keys = get_keys_down();
struct controller_data held = get_keys_held();

if (keys.c[0].A) {
    // A button just pressed
}

if (held.c[0].up) {
    // D-pad up being held
}

// Analog stick
int stick_x = keys.c[0].x; // -128 to 127
int stick_y = keys.c[0].y;
```

## Code Style Guidelines

### General Principles
- Follow standard C best practices
- Prioritize readability over cleverness
- Use descriptive variable names
- Comment complex algorithms or hardware interactions
- Keep functions focused and reasonably sized

### Naming Conventions (Flexible)
- Functions: `snake_case` or `camelCase` - be consistent within modules
- Structs: `snake_case` with `_t` suffix (e.g., `player_state_t`)
- Constants: `UPPER_SNAKE_CASE`
- Globals: Prefix with `g_` if used (minimize global usage)

### File Organization
- One major system per file (e.g., `graphics.c`, `audio.c`, `player.c`)
- Header files for public interfaces
- Keep `main.c` as entry point and main loop only

## Asset Pipeline

### Filesystem Access
Libdragon uses `rom:/` prefix for ROM filesystem:

```c
// Correct
FILE *f = fopen("rom:/data/level1.dat", "r");

// Wrong - no rom:/ prefix
FILE *f = fopen("data/level1.dat", "r");
```

### Asset Formats
- **Sprites**: `.sprite` format (use `mksprite` tool)
- **Audio**: `.wav64` for SFX, `.xm64` for music (use `audioconv64`)
- **Fonts**: TrueType `.ttf` files supported in preview branch
- **Models**: N64 model formats (depends on rendering approach)

### Adding Assets
1. Place source assets in appropriate directories
2. Add conversion steps to Makefile if needed
3. Reference with `rom:/` prefix in code

## Debugging

### Debug Builds
Use debug build for easier debugging:
```bash
make clean && make debug
```

Debug builds:
- Include debug symbols (`-g`)
- Disable optimization (`-O0`)
- Produce larger ROMs
- Code structure matches source closely

### Debugging Tools
- **Ares Emulator**: Best debugging features
- **GDB**: Can debug with `mips64-elf-gdb` (advanced)
- **Print Debugging**: Use `console_printf()` for quick debugging

### Common Issues
1. **Crash on startup**: Check initialization order
2. **Graphics not showing**: Verify RDP attach/detach order
3. **Audio stuttering**: Check buffer sizes and callback timing
4. **Controller not working**: Ensure `controller_init()` called before `controller_scan()`

## Testing and Validation

### Pre-Commit Checklist
- [ ] Code compiles without warnings
- [ ] Tested on Ares emulator
- [ ] No memory leaks (if using dynamic allocation)
- [ ] Commit message follows policy (no AI mentions, no emojis)

### Performance Considerations
- N64 runs at 60 FPS (NTSC) or 50 FPS (PAL)
- Target 16.67ms per frame (NTSC)
- Profile with emulator's performance tools
- Watch for RDP command buffer overflow

## Resources

### Official Documentation
- Libdragon API Docs: https://libdragon.dev/
- N64brew Wiki: Community knowledge base
- N64 Hardware Specs: CPU, RCP, memory maps

### Learning Resources
- N64 Squid tutorials (referenced in project setup)
- Example projects in Libdragon repo
- N64brew Discord community

### Development Tools
- **Ares**: Recommended emulator for development/debugging
- **Make**: Build automation
- **GDB**: Advanced debugging (optional)

## Getting Help

When asking for development help:
1. Provide specific error messages or behavior
2. Mention what you've already tried
3. Include relevant code snippets
4. Specify if issue is build-time or runtime

This project uses Libdragon preview branch - check if issues are preview-specific vs. stable branch.
