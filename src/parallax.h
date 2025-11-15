#ifndef PARALLAX_H
#define PARALLAX_H

#include <libdragon.h>

/**
 * Initialize the parallax system
 */
void parallax_init(void);

/**
 * Add a parallax layer
 *
 * @param sprite Sprite to use for this layer (should tile horizontally)
 * @param scroll_speed Pixels per second to scroll (can be negative for reverse)
 * @param y_position Vertical position on screen
 */
void parallax_add_layer(sprite_t *sprite, float scroll_speed, int y_position);

/**
 * Update parallax scrolling
 *
 * @param delta_time Time since last frame in seconds
 */
void parallax_update(float delta_time);

/**
 * Render all parallax layers
 *
 * @param disp Display context to render to
 */
void parallax_render(display_context_t disp);

/**
 * Change the scroll speed of a specific layer
 *
 * @param layer_index Index of layer (0-based)
 * @param new_speed New scroll speed in pixels per second
 */
void parallax_set_speed(int layer_index, float new_speed);

/**
 * Reset all layer offsets to 0
 */
void parallax_reset(void);

/**
 * Clean up parallax system
 */
void parallax_cleanup(void);

#endif // PARALLAX_H
