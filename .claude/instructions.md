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

## Resources and Examples

### Official Documentation
- **Libdragon API Docs**: https://libdragon.dev/ - Complete API reference with examples
- **Libdragon GitHub**: https://github.com/DragonMinded/libdragon - SDK source code and official examples
- **N64brew Wiki**: https://n64brew.dev/wiki/Main_Page - Community knowledge base with tutorials and technical specs
- **N64 Squid**: https://n64squid.com/ - Modern N64 development tutorials

### Essential Libraries

#### Tiny3D - 3D Graphics Library
- **Repository**: https://github.com/HailToDodongo/tiny3d
- **Purpose**: Simplified 3D rendering on N64 using Libdragon
- **Use Cases**: 3D games, tech demos with models and cameras
- **Key Features**: Model loading, camera system, lighting, texture mapping

Many successful homebrew games use Tiny3D alongside Libdragon for 3D graphics.

### Complete Games and Applications

These are production-quality projects demonstrating real-world Libdragon usage:

#### Games Using Libdragon + Tiny3D
- **Driving Strikers 64**: https://github.com/SpookyIluha/Driving-Strikers-64 - Vehicle combat game
- **CounterEmotion Bar**: https://github.com/SpookyIluha/CounterEmotion-Bar - Bar management game
- **Space Waves N64**: https://github.com/SpookyIluha/SpaceWavesN64 - Space shooter
- **Hungover**: https://github.com/RosieSapphire/Hungover - First-person exploration

#### Games Using Libdragon (2D/Other)
- **DDLC64**: https://github.com/SpookyIluha/DDLC64-LibdragonVNE - Visual novel engine demo
- **Super Haxagon64**: https://github.com/SpookyIluha/Super-Haxagon64 - Fast-paced arcade game
- **Starship Madness 64**: https://github.com/SpookyIluha/StarshipMadness64 - Space combat
- **FNaF64**: https://github.com/RosieSapphire/FNaF64 - Horror game implementation

### Tech Demos

Focused demonstrations of specific techniques:

- **BrewChristmas**: https://github.com/SpookyIluha/BrewChristmas - Tiny3D rendering showcase
- **Brew Skydome N64**: https://github.com/SpookyIluha/Brew-SkydomeN64 - Skydome/skybox rendering
- **BrewReality**: https://github.com/SpookyIluha/BrewReality - Advanced rendering techniques

### Community Projects

- **N64brew Game Jam 2024**: https://github.com/n64brew/N64brew-GameJam2024 - Collection of jam entries (mini-games)
- **N64brew Discord**: Active development community for questions and collaboration

### Development Tools
- **Ares**: Recommended emulator for development/debugging (best accuracy and debugging features)
- **Make**: Build automation (required)
- **GDB**: Advanced debugging with `mips64-elf-gdb` (optional)
- **mksprite**: Sprite conversion tool (included with Libdragon)
- **audioconv64**: Audio asset conversion (included with Libdragon)

### Common Code Patterns from Community Projects

#### 3D Rendering Setup (Tiny3D Pattern)
Many projects use Tiny3D for 3D graphics. Common initialization:

```c
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>

// Initialize Tiny3D
t3d_init((T3DInitParams){
    .screen_width = 320,
    .screen_height = 240,
    .font = NULL
});

// Load 3D model
T3DModel *model = t3d_model_load("rom:/models/player.t3dm");

// Main loop with 3D rendering
while (1) {
    t3d_frame_start();

    // Set up camera
    t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(60.0f), 10.0f, 200.0f);
    t3d_viewport_look_at(&viewport, &cam_pos, &cam_target, &cam_up);

    // Draw model
    t3d_model_draw(model);

    rdp_detach_display();
    display_show(disp);
}
```

#### Sprite Batch Rendering (2D Games Pattern)
For 2D games, batch sprite rendering is common:

```c
// Load sprite sheet
sprite_t *sprites = sprite_load("rom:/gfx/spritesheet.sprite");

// In render loop
rdp_attach_display(disp);
rdp_enable_texture_copy();

// Draw multiple sprites
for (int i = 0; i < entity_count; i++) {
    if (entities[i].active) {
        rdp_draw_sprite(
            entities[i].x,
            entities[i].y,
            entities[i].sprite,
            MIRROR_DISABLED
        );
    }
}

rdp_detach_display();
```

#### Audio System Setup (Common Pattern)
Most games use this audio initialization pattern:

```c
#include <audio.h>
#include <wav64.h>

// Initialize
audio_init(44100, 4);

// Load and play background music
xm64player_t music;
xm64player_open(&music, "rom:/music/bgm.xm64");
xm64player_play(&music, 0);  // Channel 0 for music

// Load sound effects
wav64_t jump_sfx;
wav64_open(&jump_sfx, "rom:/sfx/jump.wav64");

// Play SFX when needed
if (player_jumped) {
    wav64_play(&jump_sfx, 31);  // Channel 31 for SFX
}
```

#### Game State Management (Common Pattern)
State machines are standard for game flow:

```c
typedef enum {
    GAME_STATE_MENU,
    GAME_STATE_PLAYING,
    GAME_STATE_PAUSED,
    GAME_STATE_GAME_OVER
} game_state_t;

game_state_t current_state = GAME_STATE_MENU;

void update(float delta_time) {
    switch (current_state) {
        case GAME_STATE_MENU:
            update_menu();
            if (start_pressed) current_state = GAME_STATE_PLAYING;
            break;

        case GAME_STATE_PLAYING:
            update_gameplay(delta_time);
            if (pause_pressed) current_state = GAME_STATE_PAUSED;
            break;

        case GAME_STATE_PAUSED:
            if (start_pressed) current_state = GAME_STATE_PLAYING;
            break;

        case GAME_STATE_GAME_OVER:
            if (start_pressed) current_state = GAME_STATE_MENU;
            break;
    }
}
```

#### Delta Time Calculation (Frame-Independent Movement)
For smooth gameplay across different frame rates:

```c
#include <timer.h>

unsigned long long last_time = 0;

void game_loop(void) {
    while (1) {
        // Calculate delta time
        unsigned long long current_time = get_ticks();
        float delta_time = (float)(current_time - last_time) / TICKS_PER_SECOND;
        last_time = current_time;

        // Use delta time for movement
        player.x += player.velocity_x * delta_time;
        player.y += player.velocity_y * delta_time;

        // Render
        render_frame();
    }
}
```

### Learning Path Recommendations

**Beginner (Console and 2D)**
1. Start with console output (current project state)
2. Add sprite rendering with RDP
3. Implement controller input and simple movement
4. Add sound effects with WAV64
5. Study: Super Haxagon64, Starship Madness 64

**Intermediate (Advanced 2D)**
1. Implement sprite batching and layers
2. Add background music with XM64
3. Create particle systems
4. Build state machine for game flow
5. Study: DDLC64, N64brew Game Jam entries

**Advanced (3D Graphics)**
1. Learn Tiny3D library basics
2. Implement camera system
3. Load and render 3D models
4. Add lighting and textures
5. Study: BrewChristmas, Brew Skydome, Driving Strikers 64

### Where to Find Answers

When you need help:
1. **Check official examples**: Libdragon repo has examples for all major features
2. **Study similar games**: Find a game doing what you want and examine the code
3. **Ask here with specifics**: Provide error messages, code snippets, what you've tried
4. **N64brew Discord**: Active community for real-time help
5. **N64brew Wiki**: Technical documentation and tutorials

## Getting Help

When asking for development help:
1. Provide specific error messages or behavior
2. Mention what you've already tried
3. Include relevant code snippets
4. Specify if issue is build-time or runtime

This project uses Libdragon preview branch - check if issues are preview-specific vs. stable branch.
