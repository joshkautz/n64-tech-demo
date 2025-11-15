# Libdragon API Reference

Comprehensive reference for Libdragon SDK APIs covering graphics, audio, input, and core systems.

**Note:** This is a quick reference based on preview branch. For complete documentation, see https://libdragon.dev/

## Table of Contents
- [Display Management](#display-management)
- [Graphics (RDP)](#graphics-rdp)
- [Sprites](#sprites)
- [Fonts and Text](#fonts-and-text)
- [Audio System](#audio-system)
- [Controller Input](#controller-input)
- [Timing](#timing)
- [Filesystem](#filesystem)
- [Memory](#memory)
- [DMA](#dma)

---

## Display Management

### Initialization
```c
#include <libdragon.h>

// Resolution options
resolution_t resolution = RESOLUTION_320x240;  // Standard
// Also: RESOLUTION_640x480, RESOLUTION_256x240, etc.

// Bit depth
bitdepth_t depth = DEPTH_16_BPP;  // 16-bit color (recommended)
// Also: DEPTH_32_BPP (slower)

// Number of buffers (2 or 3)
uint32_t num_buffers = 2;  // Double buffering
// Or: 3 for triple buffering (smoother but more memory)

// Gamma correction
gamma_t gamma = GAMMA_NONE;
// Also: GAMMA_CORRECT, GAMMA_CORRECT_DITHER

// Anti-aliasing
antialias_t aa = FILTERS_RESAMPLE;
// Also: FILTERS_DISABLED, FILTERS_DEDITHER, FILTERS_RESAMPLE_ANTIALIAS

display_init(resolution, depth, num_buffers, gamma, aa);
```

### Frame Rendering Loop
```c
while (1)
{
    // Get display context for current frame
    display_context_t disp = 0;
    while (!(disp = display_lock()));  // Wait for available buffer

    // Clear screen (optional)
    graphics_fill_screen(disp, 0x000000FF);  // Fill with black

    // All drawing operations go here
    // ...

    // Show completed frame
    display_show(disp);
}
```

### Display Functions
```c
// Lock display for rendering (returns 0 if no buffer available)
display_context_t display_lock(void);

// Show completed frame
void display_show(display_context_t disp);

// Get display context without locking (polling)
display_context_t display_get(void);

// Close display
void display_close(void);
```

---

## Graphics (RDP)

### RDP Initialization
```c
#include <rdp.h>

// Initialize RDP (Reality Display Processor)
void rdp_init(void);

// Close RDP
void rdp_close(void);
```

### RDP Attachment
```c
// Attach RDP to display context (required before drawing)
void rdp_attach_display(display_context_t disp);

// Detach RDP (required after drawing)
void rdp_detach_display(void);

// Sync - wait for RDP to finish
void rdp_sync(sync_t sync);
// sync options: SYNC_FULL, SYNC_PIPE, SYNC_LOAD
```

### RDP Drawing Modes
```c
// Enable texture copy mode (for sprites)
void rdp_enable_texture_copy(void);

// Enable primitive rendering
void rdp_enable_primitive_fill(void);

// Enable blend mode
void rdp_enable_blend_fill(void);

// Set blend color
void rdp_set_blend_color(uint32_t color);
```

### RDP Primitive Drawing
```c
// Draw filled rectangle
void rdp_draw_filled_rectangle(int x, int y, int width, int height);

// Draw textured rectangle (with sprite)
void rdp_draw_textured_rectangle(
    int tx, int ty,           // Texture coordinates
    int x, int y,             // Screen position
    int width, int height,    // Size
    mirror_t mirror           // Mirroring: MIRROR_DISABLED, MIRROR_X, MIRROR_Y, MIRROR_XY
);

// Set primitive color
void rdp_set_primitive_color(uint32_t color);
```

### RDP Texture Loading
```c
// Load texture data to TMEM (texture memory)
void rdp_load_texture(
    uint32_t texslot,    // Texture slot (0-7)
    uint32_t texloc,     // TMEM location
    mirror_t mirror,     // Mirroring
    sprite_t *sprite     // Sprite to load
);

// Simpler sprite drawing
void rdp_draw_sprite(int x, int y, sprite_t *sprite, mirror_t mirror);
```

### Graphics Helpers
```c
#include <graphics.h>

// Fill entire screen with color
void graphics_fill_screen(display_context_t disp, uint32_t color);

// Draw single pixel
void graphics_draw_pixel(display_context_t disp, int x, int y, uint32_t color);

// Draw line
void graphics_draw_line(
    display_context_t disp,
    int x1, int y1,
    int x2, int y2,
    uint32_t color
);

// Draw box (outline)
void graphics_draw_box(
    display_context_t disp,
    int x, int y,
    int width, int height,
    uint32_t color
);

// Draw filled box
void graphics_draw_box_trans(
    display_context_t disp,
    int x, int y,
    int width, int height,
    uint32_t color
);

// Draw circle (outline)
void graphics_draw_circle(
    display_context_t disp,
    int x, int y,
    int radius,
    uint32_t color
);
```

### Color Format
```c
// 16-bit color (RGBA 5551)
// Format: RRRRRGGGGGBBBBBA
uint32_t color16 = graphics_make_color(r, g, b, a);  // r,g,b,a = 0-255
// Returns packed 16-bit color

// 32-bit color (RGBA 8888)
uint32_t color32 = graphics_convert_color(color16);
```

---

## Sprites

### Sprite Structure
```c
typedef struct sprite_s
{
    uint32_t width;      // Width in pixels
    uint32_t height;     // Height in pixels
    uint32_t bitdepth;   // Bits per pixel
    uint32_t format;     // Texture format
    uint32_t hslices;    // Horizontal slices
    uint32_t vslices;    // Vertical slices
    void *data;          // Texture data
} sprite_t;
```

### Loading Sprites
```c
// Load sprite from ROM filesystem
sprite_t *sprite_load(const char *fn);
// Example: sprite_t *player = sprite_load("rom:/gfx/player.sprite");

// Free sprite
void sprite_free(sprite_t *sprite);
```

### Drawing Sprites
```c
// Draw sprite using graphics subsystem (software)
void graphics_draw_sprite(
    display_context_t disp,
    int x, int y,
    sprite_t *sprite
);

// Draw sprite using RDP (hardware-accelerated)
void rdp_draw_sprite(
    int x, int y,
    sprite_t *sprite,
    mirror_t mirror  // MIRROR_DISABLED, MIRROR_X, MIRROR_Y, MIRROR_XY
);

// Draw scaled sprite
void rdp_draw_sprite_scaled(
    int x, int y,
    float x_scale,
    float y_scale,
    sprite_t *sprite,
    mirror_t mirror
);
```

### Sprite Utilities
```c
// Create sprite dynamically
sprite_t *sprite_create(uint32_t width, uint32_t height, uint32_t bitdepth);

// Get pixel from sprite
uint32_t sprite_get_pixel(sprite_t *sprite, int x, int y);

// Set pixel in sprite
void sprite_set_pixel(sprite_t *sprite, int x, int y, uint32_t color);
```

---

## Fonts and Text

### Console (Simple Text Output)
```c
#include <console.h>

// Initialize console
void console_init(void);

// Set console render mode
void console_set_render_mode(int mode);
// Modes: RENDER_MANUAL, RENDER_AUTOMATIC

// Clear console
void console_clear(void);

// Render console to display (if RENDER_MANUAL)
void console_render(display_context_t disp);

// Close console
void console_close(void);

// Print to console (uses printf syntax)
printf("Score: %d\n", score);
```

### TrueType Fonts (Preview Branch)
```c
#include <rdpq_font.h>

// Load font from TTF file
rdpq_font_t *font = rdpq_font_load("rom:/fonts/arial.ttf");

// Set font size
rdpq_font_style(font, size, NULL);

// Draw text with RDP
rdpq_text_print(
    font,
    ALIGN_LEFT,  // Or: ALIGN_CENTER, ALIGN_RIGHT
    x, y,
    "Hello World"
);

// Draw formatted text
rdpq_text_printf(
    font,
    ALIGN_LEFT,
    x, y,
    "Score: %d",
    score
);

// Free font
rdpq_font_free(font);
```

---

## Audio System

### Audio Initialization
```c
#include <audio.h>

// Initialize audio system
void audio_init(int frequency, int numbuffers);
// Common: audio_init(44100, 4);  // 44.1kHz, 4 buffers

// Close audio
void audio_close(void);
```

### Audio Callbacks (Low-level)
```c
// Write audio samples in callback
void audio_write(short *buffer);

// Get writable audio buffer
short *audio_write_begin(void);

// Finish writing audio buffer
void audio_write_end(void);
```

### WAV64 (Sound Effects)
```c
#include <wav64.h>

// Open WAV64 file
wav64_t sfx;
wav64_open(&sfx, "rom:/sfx/jump.wav64");

// Play on channel (0-63, recommend 31 for SFX)
wav64_play(&sfx, 31);

// Set volume (0.0 to 1.0)
wav64_set_volume(&sfx, 0.8f);

// Check if playing
bool playing = wav64_is_playing(&sfx);

// Stop playback
wav64_stop(&sfx);

// Close WAV64
wav64_close(&sfx);
```

### XM64 (Module Music)
```c
#include <xm64player.h>

// Open XM64 module
xm64player_t music;
xm64player_open(&music, "rom:/music/theme.xm64");

// Play on channel (recommend 0 for music)
xm64player_play(&music, 0);

// Set volume
xm64player_set_vol(&music, 0.5f);  // 0.0 to 1.0

// Stop playback
xm64player_stop(&music);

// Close player
xm64player_close(&music);
```

### Audio Channels
- Channels 0-30: General purpose (music, ambient)
- Channel 31: Recommended for sound effects
- Total: 32 channels

---

## Controller Input

### Controller Initialization
```c
#include <controller.h>

// Initialize controller system
void controller_init(void);

// Scan controllers (call once per frame)
void controller_scan(void);
```

### Reading Input
```c
// Get controller data structure
struct controller_data keys_down = get_keys_down();
struct controller_data keys_up = get_keys_up();
struct controller_data keys_held = get_keys_held();

// Controller data is array of 4 controllers
// keys.c[0] = Player 1
// keys.c[1] = Player 2, etc.
```

### Controller Data Structure
```c
struct controller_data
{
    struct SI_condat
    {
        // Digital buttons (bool)
        uint16_t A : 1;
        uint16_t B : 1;
        uint16_t Z : 1;
        uint16_t start : 1;
        uint16_t up : 1;
        uint16_t down : 1;
        uint16_t left : 1;
        uint16_t right : 1;
        uint16_t L : 1;
        uint16_t R : 1;
        uint16_t C_up : 1;
        uint16_t C_down : 1;
        uint16_t C_left : 1;
        uint16_t C_right : 1;

        // Analog stick (-128 to 127)
        int8_t x;
        int8_t y;
    } c[4];
};
```

### Input Examples
```c
// Check button press
if (keys_down.c[0].A) {
    player_jump();
}

// Check button held
if (keys_held.c[0].Z) {
    player_crouch();
}

// Check button release
if (keys_up.c[0].B) {
    player_stop_running();
}

// Read analog stick
int stick_x = keys_held.c[0].x;  // -128 to 127
int stick_y = keys_held.c[0].y;

// Apply deadzone
#define DEADZONE 10
if (abs(stick_x) > DEADZONE) {
    player_move_x(stick_x);
}

// Check D-pad
if (keys_held.c[0].up) {
    menu_move_up();
}
```

### Accessory Detection
```c
// Get controller accessories for each port
struct controller_data accessories = get_accessories_present();

if (accessories.c[0].controller_pak) {
    // Controller pak inserted in port 1
}

if (accessories.c[0].rumble_pak) {
    // Rumble pak inserted in port 1
}
```

### Rumble Support
```c
// Start rumble on controller
void rumble_start(int controller);  // 0-3

// Stop rumble
void rumble_stop(int controller);
```

---

## Timing

### Timer System
```c
#include <timer.h>

// Initialize timer subsystem
void timer_init(void);

// Close timer
void timer_close(void);

// Get current ticks (incrementing counter)
unsigned long long get_ticks(void);

// Get ticks per second
long long TICKS_PER_SECOND;

// Calculate elapsed time
unsigned long long start = get_ticks();
// ... do work ...
unsigned long long elapsed = get_ticks() - start;
float seconds = (float)elapsed / TICKS_PER_SECOND;
```

### Timing Utilities
```c
// Get ticks from microseconds
unsigned long long ticks_from_us(unsigned long long us);

// Get ticks from milliseconds
unsigned long long ticks_from_ms(unsigned long long ms);

// Convert ticks to microseconds
unsigned long long ticks_to_us(unsigned long long ticks);

// Convert ticks to milliseconds
unsigned long long ticks_to_ms(unsigned long long ticks);
```

### Frame Timing Example
```c
#define TARGET_FPS 60
#define FRAME_TICKS (TICKS_PER_SECOND / TARGET_FPS)

unsigned long long last_frame = get_ticks();

while (1)
{
    unsigned long long now = get_ticks();
    unsigned long long delta = now - last_frame;

    if (delta >= FRAME_TICKS)
    {
        // Run game logic
        update_game(delta);
        render_frame();

        last_frame = now;
    }
}
```

---

## Filesystem

### DFS (DragonFS - ROM Filesystem)
```c
#include <dfs.h>

// Initialize filesystem (call once at startup)
int dfs_init(uint32_t base_fs_loc);
// Usually: dfs_init(DFS_DEFAULT_LOCATION);

// Open file (uses standard C FILE*)
FILE *fp = fopen("rom:/data/level1.dat", "r");

// Read/write using standard C functions
fread(buffer, 1, size, fp);
fwrite(buffer, 1, size, fp);

// Close file
fclose(fp);
```

### File Operations
```c
// Check if file exists
int file_exists(const char *path);

// Get file size
int dfs_size(const char *path);

// Directory operations (if supported)
dir_t dir;
int dfs_dir_findfirst("rom:/gfx", &dir);
while (dfs_dir_findnext(&dir) == 0) {
    printf("File: %s\n", dir.d_name);
}
```

### ROM Filesystem Path
All ROM files use `rom:/` prefix:
```c
// Correct
sprite_load("rom:/sprites/player.sprite");
fopen("rom:/data/config.txt", "r");

// Wrong (will fail)
sprite_load("sprites/player.sprite");
fopen("data/config.txt", "r");
```

---

## Memory

### Memory Layout
N64 has 4MB RAM by default (8MB with Expansion Pak):
- RDRAM: 0x80000000 - 0x803FFFFF (4MB)
- Expanded: 0x80000000 - 0x807FFFFF (8MB)

### Memory Allocation
```c
#include <malloc.h>

// Standard C allocation
void *ptr = malloc(size);
void *ptr = calloc(count, size);
void *ptr = realloc(ptr, new_size);
free(ptr);

// Aligned allocation (for DMA)
void *ptr = memalign(alignment, size);
```

### Memory Constraints
- Total RAM: 4MB (8MB with expansion)
- Stack size: Limited, avoid deep recursion
- Use static allocation for large buffers when possible
- Minimize dynamic allocation in game loop

### Memory Tips
```c
// Check alignment for DMA
#define IS_ALIGNED(ptr, align) (((uintptr_t)(ptr) & ((align) - 1)) == 0)

if (!IS_ALIGNED(buffer, 8)) {
    // Buffer not aligned for DMA
}

// Allocate aligned buffer
void *buffer = memalign(8, size);  // 8-byte aligned
```

---

## DMA

### DMA Operations
```c
#include <dma.h>

// DMA read from ROM to RAM
void dma_read(void *ram_address, unsigned long rom_address, unsigned long len);

// DMA write from RAM to ROM (cart writes, if supported)
void dma_write(void *ram_address, unsigned long rom_address, unsigned long len);

// Wait for DMA to complete
void dma_wait(void);
```

### DMA Requirements
- Source and destination must be 8-byte aligned
- Length must be multiple of 8 bytes
- RAM address must be in RDRAM (0x80000000+)
- ROM address is physical (use & with 0x1FFFFFFF)

### DMA Example
```c
// Allocate aligned buffer
void *buffer = memalign(8, 1024);

// Read from ROM
dma_read(buffer, 0x1000, 1024);

// Wait for completion
dma_wait();

// Use buffer
process_data(buffer);

// Free buffer
free(buffer);
```

---

## Advanced Topics

### RSP (Reality Signal Processor)
The RSP handles custom microcode for specialized tasks:
- Audio processing
- Vertex transformation
- Custom effects

**Note:** RSP programming is advanced. Most developers use Libdragon's built-in RSP microcodes.

### Display Lists
For complex graphics, build display lists:
```c
// Coming in preview branch updates
// Display lists allow pre-recording RDP commands for efficiency
```

### Compression
```c
// Libdragon supports compressed assets
// Use tools to compress sprites, audio, etc.
```

---

## Common Patterns

### Game Loop Structure
```c
int main(void)
{
    // Initialize
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE, FILTERS_RESAMPLE);
    rdp_init();
    controller_init();
    timer_init();
    audio_init(44100, 4);
    dfs_init(DFS_DEFAULT_LOCATION);

    // Load assets
    sprite_t *player_sprite = sprite_load("rom:/gfx/player.sprite");

    // Game loop
    while (1)
    {
        // Input
        controller_scan();
        struct controller_data keys = get_keys_down();

        // Update
        update_game(keys);

        // Render
        display_context_t disp = 0;
        while (!(disp = display_lock()));

        rdp_attach_display(disp);
        graphics_fill_screen(disp, 0x000000FF);

        render_game(disp);

        rdp_detach_display();
        display_show(disp);
    }

    return 0;
}
```

### State Machine
```c
typedef enum {
    STATE_MENU,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAME_OVER
} game_state_t;

game_state_t state = STATE_MENU;

void update_game(struct controller_data keys)
{
    switch (state)
    {
        case STATE_MENU:
            update_menu(keys);
            break;
        case STATE_PLAYING:
            update_gameplay(keys);
            break;
        case STATE_PAUSED:
            update_pause_menu(keys);
            break;
        case STATE_GAME_OVER:
            update_game_over(keys);
            break;
    }
}
```

### Entity Management
```c
#define MAX_ENTITIES 64

typedef struct {
    float x, y;
    float vx, vy;
    sprite_t *sprite;
    bool active;
} entity_t;

entity_t entities[MAX_ENTITIES];

void spawn_entity(float x, float y, sprite_t *sprite)
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        if (!entities[i].active)
        {
            entities[i].x = x;
            entities[i].y = y;
            entities[i].sprite = sprite;
            entities[i].active = true;
            break;
        }
    }
}

void update_entities(void)
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        if (entities[i].active)
        {
            entities[i].x += entities[i].vx;
            entities[i].y += entities[i].vy;
        }
    }
}
```

---

## Performance Tips

1. **Use RDP for graphics**: Hardware-accelerated, much faster than software
2. **Minimize state changes**: Group similar draw calls together
3. **Texture memory**: TMEM is only 4KB - manage carefully
4. **Double buffering**: Minimum recommended (triple for smoother)
5. **Profile**: Use emulator tools to find bottlenecks
6. **Static allocation**: Faster than dynamic malloc/free in loop
7. **Audio buffers**: More buffers = more latency but smoother playback

---

## Debugging Tips

### Printf Debugging
```c
// Console output (visible in emulator)
printf("Player pos: %d, %d\n", player.x, player.y);

// Debug assertions
#ifdef DEBUG
    assert(player.health > 0);
#endif
```

### Common Errors
- **Black screen**: Check RDP attach/detach order
- **Crash on load**: Check file paths use `rom:/` prefix
- **Controller not working**: Forgot `controller_scan()`
- **No audio**: Check `audio_init()` called, correct channels used
- **Texture artifacts**: Check TMEM size limits, texture format

---

This reference covers the most commonly used Libdragon APIs. For complete documentation and advanced features, see https://libdragon.dev/
