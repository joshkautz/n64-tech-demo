#include <stdio.h>
#include <libdragon.h>
#include "parallax.h"

int main(void)
{
    /* Initialize display */
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE, FILTERS_RESAMPLE);

    /* Initialize RDP for hardware-accelerated graphics */
    rdp_init();

    /* Initialize timer for delta time calculations */
    timer_init();

    /* Initialize parallax system */
    parallax_init();

    /* TODO: Load sprite assets and add parallax layers
     * Example (once you have sprite files):
     *
     * sprite_t *bg_far = sprite_load("rom:/gfx/background_far.sprite");
     * sprite_t *bg_mid = sprite_load("rom:/gfx/background_mid.sprite");
     * sprite_t *bg_near = sprite_load("rom:/gfx/background_near.sprite");
     *
     * parallax_add_layer(bg_far, 20.0f, 0);    // Slowest layer, back
     * parallax_add_layer(bg_mid, 50.0f, 80);   // Medium speed, middle
     * parallax_add_layer(bg_near, 100.0f, 160); // Fastest layer, front
     */

    /* Initialize console for text overlay */
    console_init();
    console_set_render_mode(RENDER_MANUAL);

    /* Track time for delta calculations */
    unsigned long long last_time = get_ticks();

    /* Main loop */
    while(1)
    {
        /* Calculate delta time */
        unsigned long long current_time = get_ticks();
        float delta_time = (float)(current_time - last_time) / TICKS_PER_SECOND;
        last_time = current_time;

        /* Cap delta time to prevent large jumps */
        if (delta_time > 0.1f) delta_time = 0.1f;

        /* Update parallax scrolling */
        parallax_update(delta_time);

        /* Get display buffer */
        display_context_t disp = 0;
        while(!(disp = display_lock()));

        /* Clear screen to black */
        graphics_fill_screen(disp, 0x000000FF);

        /* Render parallax layers */
        parallax_render(disp);

        /* Render console text overlay */
        console_clear();
        printf("\n N64 Tech Demo - Parallax Scrolling\n");
        printf("\n FPS: %.1f\n", delta_time > 0 ? 1.0f / delta_time : 60.0f);
        printf("\n Add sprite assets to see parallax!\n");
        console_render(disp);

        /* Show completed frame */
        display_show(disp);
    }

    return 0;
}
