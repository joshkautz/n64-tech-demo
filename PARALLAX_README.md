# Parallax Scrolling Implementation

This document explains the parallax scrolling system added to the N64 tech demo.

## What Was Added

### New Files

1. **src/parallax.c** - Parallax system implementation
2. **src/parallax.h** - Public API header
3. **GRAPHICS_GUIDE.md** - Complete graphics and asset guide

### Modified Files

1. **src/main.c** - Updated to use display and parallax system
2. **Makefile** - Added parallax.c to build

## How It Works

The parallax system creates a scrolling background effect with multiple layers moving at different speeds, creating an illusion of depth.

### Architecture

```
parallax_init()
  ↓
parallax_add_layer() × N   (Add multiple layers)
  ↓
Main Loop:
  ├─ parallax_update(delta_time)   (Update scroll offsets)
  └─ parallax_render(disp)         (Draw all layers to screen)
```

### Key Features

- **Hardware accelerated**: Uses RDP for fast sprite rendering
- **Seamless tiling**: Automatically wraps sprites for infinite scrolling
- **Delta time based**: Smooth scrolling independent of frame rate
- **Multiple layers**: Support for up to 4 parallax layers
- **Configurable**: Each layer has independent speed and position

## Building and Testing

### Step 1: Open in Dev Container

In VS Code:
1. Press `Cmd+Shift+P`
2. Select "Dev Containers: Reopen in Container"
3. Wait for container to start

### Step 2: Build ROM

```bash
# In VS Code terminal (inside container)
make clean && make
```

Or use the keyboard shortcut:
- Press `Cmd+Shift+B`
- Select "Build ROM"

### Step 3: Test ROM

1. ROM will be built as `n64-tech-demo.z64`
2. In macOS Finder, navigate to project directory
3. Double-click `n64-tech-demo.z64` to open in Ares emulator

**What you'll see:**
- Black screen
- Console text overlay showing "N64 Tech Demo - Parallax Scrolling"
- FPS counter
- Message: "Add sprite assets to see parallax!"

The parallax system is working, but you need to add sprite assets to see it in action.

## Adding Graphics Assets

### Quick Test (Solid Color Layers)

For testing without creating artwork, you can modify `main.c` to draw colored rectangles as placeholder layers.

Add this function before `main()`:

```c
/* Create a simple colored sprite in memory for testing */
sprite_t* create_test_sprite(int width, int height, uint32_t color)
{
    sprite_t *sprite = sprite_create(width, height, DEPTH_16_BPP);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            sprite_set_pixel(sprite, x, y, color);
        }
    }

    return sprite;
}
```

Then in `main()`, replace the TODO section with:

```c
/* Create test sprites with solid colors */
sprite_t *bg_far = create_test_sprite(320, 80, 0x1a1a2eFF);   // Dark blue
sprite_t *bg_mid = create_test_sprite(320, 80, 0x16213eFF);   // Medium blue
sprite_t *bg_near = create_test_sprite(320, 80, 0x0f3460FF);  // Bright blue

parallax_add_layer(bg_far, 20.0f, 0);      // Slow, top
parallax_add_layer(bg_mid, 50.0f, 80);     // Medium, middle
parallax_add_layer(bg_near, 100.0f, 160);  // Fast, bottom
```

### Using Real Sprite Assets

See **GRAPHICS_GUIDE.md** for complete instructions on:
- Creating tileable images
- Converting to N64 sprite format with `mksprite`
- Setting up asset pipeline in Makefile
- Loading sprites from ROM filesystem

## API Reference

### parallax_init()

Initialize the parallax system. Call once at startup.

```c
void parallax_init(void);
```

### parallax_add_layer()

Add a scrolling layer.

```c
void parallax_add_layer(
    sprite_t *sprite,     // Sprite to render (should tile horizontally)
    float scroll_speed,   // Pixels per second (can be negative)
    int y_position        // Vertical position on screen
);
```

**Parameters:**
- `sprite`: Pointer to loaded sprite (must be tileable on X axis)
- `scroll_speed`: Scroll rate in pixels/second
  - Positive = scroll right
  - Negative = scroll left
  - Typical values: 20-200 depending on layer depth
- `y_position`: Y coordinate where layer appears (0 = top of screen)

**Layer order:**
Add layers from back to front (furthest to nearest):
1. Background (slowest, e.g., 20 px/s)
2. Midground (medium, e.g., 50 px/s)
3. Foreground (fastest, e.g., 100 px/s)

### parallax_update()

Update scroll offsets. Call once per frame.

```c
void parallax_update(float delta_time);
```

**Parameters:**
- `delta_time`: Time since last frame in seconds

### parallax_render()

Render all layers. Call after clearing screen.

```c
void parallax_render(display_context_t disp);
```

**Parameters:**
- `disp`: Display context from `display_lock()`

### parallax_set_speed()

Change the speed of a specific layer at runtime.

```c
void parallax_set_speed(int layer_index, float new_speed);
```

**Example:**
```c
// Speed up first layer
parallax_set_speed(0, 40.0f);

// Reverse second layer
parallax_set_speed(1, -30.0f);
```

### parallax_reset()

Reset all scroll offsets to 0.

```c
void parallax_reset(void);
```

### parallax_cleanup()

Clean up parallax system (doesn't free sprites).

```c
void parallax_cleanup(void);
```

## Libdragon Graphics Functions Used

### Display System

| Function | Purpose |
|----------|---------|
| `display_init()` | Initialize video output |
| `display_lock()` | Get framebuffer for rendering |
| `display_show()` | Display completed frame |

### RDP (Hardware Graphics)

| Function | Purpose |
|----------|---------|
| `rdp_init()` | Initialize graphics hardware |
| `rdp_attach_display()` | Start rendering to buffer |
| `rdp_detach_display()` | Finish rendering |
| `rdp_enable_texture_copy()` | Enable textured sprite mode |
| `rdp_draw_sprite()` | Draw sprite at position |

### Sprites

| Function | Purpose |
|----------|---------|
| `sprite_load()` | Load sprite from ROM filesystem |
| `sprite_free()` | Free sprite memory |
| `sprite_create()` | Create sprite programmatically |
| `sprite_set_pixel()` | Set pixel in created sprite |

### Timing

| Function | Purpose |
|----------|---------|
| `timer_init()` | Initialize timer system |
| `get_ticks()` | Get current tick count |
| `TICKS_PER_SECOND` | Ticks per second constant |

## Performance

The parallax system is designed for performance:

- **Hardware accelerated**: Uses N64's RDP for sprite rendering
- **Efficient tiling**: Only draws visible portions of sprites
- **Minimal CPU usage**: Update loop is simple offset calculation
- **Frame-independent**: Uses delta time for consistent speed

**Expected performance:**
- 60 FPS with 3-4 parallax layers
- Minimal impact on frame time (<1ms per frame)

## Customization Ideas

### Variable Speed Based on Input

```c
// In main loop, after controller_scan():
struct controller_data keys = get_keys_held();

if (keys.c[0].right) {
    parallax_set_speed(0, 40.0f);   // Speed up when moving right
    parallax_set_speed(1, 100.0f);
    parallax_set_speed(2, 200.0f);
} else {
    parallax_set_speed(0, 20.0f);   // Normal speed
    parallax_set_speed(1, 50.0f);
    parallax_set_speed(2, 100.0f);
}
```

### Vertical Parallax

Modify `parallax.c` to support vertical scrolling:

```c
typedef struct {
    sprite_t *sprite;
    float scroll_speed_x;  // Horizontal speed
    float scroll_speed_y;  // Vertical speed
    float offset_x;
    float offset_y;        // Add vertical offset
    int x_position;
    int y_position;
} parallax_layer_t;
```

### Parallax with Transparency

Use sprites with alpha channel for layered effects:

```c
// Create sprite with transparency
mksprite --format RGBA32 clouds.png clouds.sprite

// Render with blending
rdp_enable_blend_fill();
rdp_draw_sprite(x, y, clouds_sprite, MIRROR_DISABLED);
```

## Next Steps

1. **Add sprite assets** - See GRAPHICS_GUIDE.md
2. **Add controller input** - Make speed interactive
3. **Add foreground elements** - Sprites that move independently
4. **Add more visual effects** - Clouds, stars, etc.
5. **Study community examples** - Check projects like BrewChristmas or SpaceWaves

## Troubleshooting

**Problem: Black screen, no graphics**
- Check that `rdp_init()` was called
- Verify sprites are loaded successfully
- Ensure `rdp_attach_display()` / `rdp_detach_display()` are paired

**Problem: Sprites don't scroll**
- Check `parallax_update()` is called in main loop
- Verify `delta_time` is calculated correctly
- Try increasing `scroll_speed` values

**Problem: Choppy scrolling**
- Ensure using delta time (not fixed timestep)
- Check frame rate is stable (60 FPS)
- Verify not using software rendering functions

**Problem: Sprites are garbled**
- Check sprite format matches display depth
- Verify `mksprite` conversion was successful
- Try rebuilding ROM completely

## Additional Resources

- **GRAPHICS_GUIDE.md** - Comprehensive graphics and asset guide
- **Libdragon API**: https://libdragon.dev/
- **N64brew Wiki**: https://n64brew.dev/wiki/Graphics
- **Example projects**: See .claude/instructions.md for community game examples
