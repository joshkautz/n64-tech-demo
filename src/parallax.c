#include "parallax.h"
#include <libdragon.h>
#include <math.h>

// Parallax layer structure
typedef struct {
    sprite_t *sprite;
    float scroll_speed;
    float offset_x;
    int y_position;
} parallax_layer_t;

#define MAX_LAYERS 4

static parallax_layer_t layers[MAX_LAYERS];
static int layer_count = 0;

void parallax_init(void)
{
    layer_count = 0;
}

void parallax_add_layer(sprite_t *sprite, float scroll_speed, int y_position)
{
    if (layer_count >= MAX_LAYERS) {
        return;
    }

    layers[layer_count].sprite = sprite;
    layers[layer_count].scroll_speed = scroll_speed;
    layers[layer_count].offset_x = 0.0f;
    layers[layer_count].y_position = y_position;
    layer_count++;
}

void parallax_update(float delta_time)
{
    for (int i = 0; i < layer_count; i++) {
        // Update scroll offset
        layers[i].offset_x += layers[i].scroll_speed * delta_time;

        // Wrap around when we've scrolled a full sprite width
        if (layers[i].sprite) {
            float sprite_width = (float)layers[i].sprite->width;
            if (layers[i].offset_x >= sprite_width) {
                layers[i].offset_x -= sprite_width;
            } else if (layers[i].offset_x < 0) {
                layers[i].offset_x += sprite_width;
            }
        }
    }
}

void parallax_render(display_context_t disp)
{
    // Attach RDP for hardware-accelerated rendering
    rdp_attach_display(disp);
    rdp_enable_texture_copy();

    for (int i = 0; i < layer_count; i++) {
        if (!layers[i].sprite) continue;

        int sprite_width = layers[i].sprite->width;
        int sprite_height = layers[i].sprite->height;
        int x_offset = (int)layers[i].offset_x;

        // For seamless scrolling, we need to draw the sprite twice:
        // Once at the current offset, and once shifted by sprite_width
        // This creates the illusion of infinite scrolling

        // Calculate how many times we need to tile horizontally to cover screen
        // N64 typical resolution is 320x240
        int screen_width = 320;
        int tiles_needed = (screen_width / sprite_width) + 2; // +2 for wraparound

        for (int tile = 0; tile < tiles_needed; tile++) {
            int x_pos = (tile * sprite_width) - x_offset;

            // Only draw if on screen
            if (x_pos + sprite_width >= 0 && x_pos < screen_width) {
                rdp_draw_sprite(
                    x_pos,
                    layers[i].y_position,
                    layers[i].sprite,
                    MIRROR_DISABLED
                );
            }
        }
    }

    rdp_detach_display();
}

void parallax_set_speed(int layer_index, float new_speed)
{
    if (layer_index >= 0 && layer_index < layer_count) {
        layers[layer_index].scroll_speed = new_speed;
    }
}

void parallax_reset(void)
{
    for (int i = 0; i < layer_count; i++) {
        layers[i].offset_x = 0.0f;
    }
}

void parallax_cleanup(void)
{
    // Note: This doesn't free sprites - caller manages sprite memory
    layer_count = 0;
}
