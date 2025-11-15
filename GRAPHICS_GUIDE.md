# Graphics and Asset Guide

This guide explains how to create and use graphics assets in your N64 tech demo.

## Quick Start: Creating Parallax Sprites

### 1. Create Your Images

**Requirements:**
- Format: PNG with transparency support
- Dimensions: Power of 2 recommended (256x64, 512x128, etc.)
- Width should be at least 320 pixels (or tileable)
- Keep file sizes reasonable (N64 has limited texture memory)

**Recommended sizes for parallax layers:**
- Background (far): 320x80 pixels
- Midground: 320x80 pixels
- Foreground (near): 320x80 pixels

**Design tips:**
- Make images tileable (left edge matches right edge)
- Use distinct colors for each layer
- Far layers: lighter, less detail, cooler colors
- Near layers: more detail, warmer/brighter colors

### 2. Convert to N64 Sprite Format

Libdragon includes the `mksprite` tool for converting images.

**Basic conversion:**
```bash
# Inside the dev container
mksprite input.png output.sprite
```

**With options:**
```bash
# 16-bit color (recommended for photos/gradients)
mksprite --format RGBA16 background.png background.sprite

# 32-bit color (higher quality, larger file)
mksprite --format RGBA32 background.png background.sprite

# With mipmaps (for scaled rendering)
mksprite --mipmap background.png background.sprite
```

### 3. Project Structure

Create an asset directory structure:

```
n64-tech-demo/
├── assets/              # Source images (PNG, etc.)
│   └── gfx/
│       ├── bg_far.png
│       ├── bg_mid.png
│       └── bg_near.png
├── filesystem/          # Converted assets (sprite files)
│   └── gfx/
│       ├── bg_far.sprite
│       ├── bg_mid.sprite
│       └── bg_near.sprite
└── src/
    └── main.c
```

### 4. Add Asset Build Step to Makefile

Update your Makefile to automatically convert sprites:

```makefile
# Asset directories
ASSETS_DIR = assets
FILESYSTEM_DIR = filesystem

# Find all PNG files in assets
ASSET_PNGS = $(wildcard $(ASSETS_DIR)/gfx/*.png)
# Convert to .sprite paths in filesystem
SPRITES = $(patsubst $(ASSETS_DIR)/%.png,$(FILESYSTEM_DIR)/%.sprite,$(ASSET_PNGS))

# Build ROM with filesystem
$(PROJECT_NAME).z64: N64_ROM_TITLE="N64 Tech Demo"
$(PROJECT_NAME).z64: $(BUILD_DIR)/$(PROJECT_NAME).elf $(SPRITES)

# Convert PNG to sprite
$(FILESYSTEM_DIR)/%.sprite: $(ASSETS_DIR)/%.png
	@mkdir -p $(dir $@)
	mksprite $< $@

# Clean sprites too
clean:
	rm -rf $(BUILD_DIR) $(PROJECT_NAME).z64 $(FILESYSTEM_DIR)
```

### 5. Update Code to Load Sprites

In `src/main.c`:

```c
#include <libdragon.h>
#include "parallax.h"

int main(void)
{
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE, FILTERS_RESAMPLE);
    rdp_init();
    timer_init();

    // Initialize ROM filesystem
    dfs_init(DFS_DEFAULT_LOCATION);

    parallax_init();

    // Load sprites from ROM filesystem
    sprite_t *bg_far = sprite_load("rom:/gfx/bg_far.sprite");
    sprite_t *bg_mid = sprite_load("rom:/gfx/bg_mid.sprite");
    sprite_t *bg_near = sprite_load("rom:/gfx/bg_near.sprite");

    // Add parallax layers (back to front)
    parallax_add_layer(bg_far, 20.0f, 0);    // Slow, top of screen
    parallax_add_layer(bg_mid, 50.0f, 80);   // Medium speed
    parallax_add_layer(bg_near, 100.0f, 160); // Fast, bottom

    // Rest of your code...
}
```

## Graphics Functions Reference

### Display Management

**Initialize display:**
```c
display_init(
    RESOLUTION_320x240,  // Resolution
    DEPTH_16_BPP,        // 16-bit color
    2,                   // Double buffering
    GAMMA_NONE,          // No gamma correction
    FILTERS_RESAMPLE     // Anti-aliasing filter
);
```

**Get display buffer:**
```c
display_context_t disp = 0;
while(!(disp = display_lock()));  // Wait for available buffer
```

**Show frame:**
```c
display_show(disp);
```

### RDP (Hardware Rendering)

**Initialize:**
```c
rdp_init();
```

**Attach to display:**
```c
rdp_attach_display(disp);
```

**Enable texture rendering:**
```c
rdp_enable_texture_copy();
```

**Draw sprite:**
```c
rdp_draw_sprite(
    x,                  // X position
    y,                  // Y position
    sprite,             // Sprite pointer
    MIRROR_DISABLED     // No mirroring
);
// Also: MIRROR_X, MIRROR_Y, MIRROR_XY
```

**Draw scaled sprite:**
```c
rdp_draw_sprite_scaled(
    x, y,               // Position
    scale_x, scale_y,   // Scale factors (1.0 = normal)
    sprite,
    MIRROR_DISABLED
);
```

**Draw filled rectangle:**
```c
rdp_set_primitive_color(0xFF0000FF);  // Red
rdp_draw_filled_rectangle(x, y, width, height);
```

**Detach RDP:**
```c
rdp_detach_display();
```

### Software Graphics (Slower, but simpler)

**Fill screen:**
```c
graphics_fill_screen(disp, 0x000000FF);  // Black (RGBA)
```

**Draw pixel:**
```c
graphics_draw_pixel(disp, x, y, 0xFF0000FF);  // Red pixel
```

**Draw line:**
```c
graphics_draw_line(disp, x1, y1, x2, y2, 0xFFFFFFFF);  // White line
```

**Draw box (outline):**
```c
graphics_draw_box(disp, x, y, width, height, 0x00FF00FF);  // Green box
```

**Draw sprite (software):**
```c
graphics_draw_sprite(disp, x, y, sprite);
```

### Sprites

**Load sprite:**
```c
sprite_t *my_sprite = sprite_load("rom:/gfx/image.sprite");
```

**Free sprite:**
```c
sprite_free(my_sprite);
```

**Sprite properties:**
```c
int width = my_sprite->width;
int height = my_sprite->height;
```

### Color Format

**Create 16-bit color:**
```c
uint32_t color = graphics_make_color(r, g, b, a);  // r,g,b,a = 0-255
// Returns RGBA 5551 format for N64
```

**Common colors:**
```c
#define COLOR_BLACK   0x000000FF
#define COLOR_WHITE   0xFFFFFFFF
#define COLOR_RED     0xFF0000FF
#define COLOR_GREEN   0x00FF00FF
#define COLOR_BLUE    0x0000FFFF
#define COLOR_YELLOW  0xFFFF00FF
#define COLOR_CYAN    0x00FFFFFF
#define COLOR_MAGENTA 0xFF00FFFF
```

## Advanced Graphics Techniques

### Sprite Batching

For multiple sprites, keep RDP attached:

```c
rdp_attach_display(disp);
rdp_enable_texture_copy();

// Draw many sprites
for (int i = 0; i < entity_count; i++) {
    rdp_draw_sprite(entities[i].x, entities[i].y, entities[i].sprite, MIRROR_DISABLED);
}

rdp_detach_display();
```

### Layered Rendering

Render in layers for correct depth:

```c
// 1. Background layer
rdp_attach_display(disp);
rdp_enable_texture_copy();
parallax_render(disp);
rdp_detach_display();

// 2. Game entities
rdp_attach_display(disp);
rdp_enable_texture_copy();
draw_player(disp);
draw_enemies(disp);
rdp_detach_display();

// 3. UI/HUD (always on top)
rdp_attach_display(disp);
draw_ui(disp);
rdp_detach_display();
```

### Screen Clear

Always clear before rendering:

```c
// Option 1: Fill with color
graphics_fill_screen(disp, 0x000000FF);  // Black

// Option 2: RDP clear (faster)
rdp_attach_display(disp);
rdp_enable_primitive_fill();
rdp_set_primitive_color(0x000000FF);
rdp_draw_filled_rectangle(0, 0, 320, 240);
rdp_detach_display();
```

## Performance Tips

1. **Use RDP, not software rendering**
   - RDP is 10-100x faster for sprite drawing
   - Only use software functions for debugging

2. **Minimize RDP state changes**
   - Batch similar operations together
   - Attach/detach RDP once per render pass if possible

3. **Texture memory (TMEM) is limited**
   - TMEM is only 4KB total
   - Large sprites are loaded in chunks
   - RDP handles this automatically, but be aware

4. **Sprite size recommendations**
   - Keep individual sprites under 256x256
   - Use power-of-2 dimensions when possible (64x64, 128x128, etc.)
   - Compress large images or split into tiles

5. **Frame rate**
   - Target 60 FPS on N64 (16.67ms per frame)
   - Use delta time for smooth animation
   - Profile with emulator tools

## Creating Test Graphics

If you don't have art assets yet, create simple test graphics:

### Using Python (Pillow)

```python
from PIL import Image, ImageDraw

# Create tileable gradient background
width, height = 320, 80
img = Image.new('RGBA', (width, height))
draw = ImageDraw.Draw(img)

# Draw gradient
for y in range(height):
    color = int(255 * (y / height))
    draw.rectangle([(0, y), (width, y)], fill=(0, 0, color, 255))

img.save('assets/gfx/bg_far.png')
```

### Using ImageMagick (command line)

```bash
# Solid color
convert -size 320x80 xc:'#3344FF' assets/gfx/bg_far.png

# Gradient
convert -size 320x80 gradient:'#1a1a2e'-'#16213e' assets/gfx/bg_far.png

# Noise pattern
convert -size 320x80 plasma:fractal assets/gfx/bg_mid.png
```

## Troubleshooting

**Problem: Sprites don't appear**
- Check `dfs_init()` is called before `sprite_load()`
- Verify file path uses `rom:/` prefix
- Confirm sprites are in `filesystem/` directory
- Check ROM was rebuilt after adding sprites

**Problem: Graphics are garbled**
- Ensure `rdp_init()` is called
- Check `rdp_attach_display()` / `rdp_detach_display()` pairing
- Verify sprite format matches display depth

**Problem: Performance is slow**
- Use RDP functions, not software graphics
- Reduce number of sprites drawn per frame
- Check sprite sizes are reasonable
- Profile with emulator

**Problem: Sprites have wrong colors**
- Check sprite format (RGBA16 vs RGBA32)
- Verify source PNG has correct color space
- Try different `mksprite` format options

## Next Steps

1. Create simple test sprites with solid colors
2. Build and test parallax scrolling
3. Experiment with scroll speeds
4. Add more complex artwork
5. Add interactivity (controller input to change speeds)

For more information, see:
- Libdragon API docs: https://libdragon.dev/
- N64brew Wiki: https://n64brew.dev/wiki/Graphics
