#include <stdio.h>
#include <libdragon.h>

int main(void)
{
    /* Initialize the console */
    console_init();
    console_set_render_mode(RENDER_AUTOMATIC);

    /* Display a welcome message */
    printf("\n");
    printf("================================\n");
    printf("     N64 Tech Demo\n");
    printf("================================\n");
    printf("\n");
    printf("Welcome to libdragon!\n");
    printf("\n");
    printf("This is a basic template for\n");
    printf("Nintendo 64 homebrew development\n");
    printf("using the libdragon SDK.\n");
    printf("\n");
    printf("Build successful!\n");
    printf("\n");
    printf("Visit libdragon.dev for docs.\n");
    printf("\n");

    /* Infinite loop - N64 programs don't exit */
    while(1)
    {
        /* Keep the N64 running */
    }

    return 0;
}
