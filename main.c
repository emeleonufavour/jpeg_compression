#include <stdio.h>
#include <stdlib.h>

void visualize_block_access(int width, int height, int start_x, int start_y, int subsample_factor)
{
    printf("Image Size: %dx%d, Block at (%d,%d) with size %d\n\n",
           width, height, start_x, start_y, subsample_factor);

    // First: Show the full image with linear indices
    printf("Full image with 1D array indices:\n");
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int idx = y * width + x;
            // Highlight block positions
            if (x >= start_x && x < start_x + subsample_factor &&
                y >= start_y && y < start_y + subsample_factor)
            {
                printf("[%2d]*", idx);
            }
            else
            {
                printf("[%2d] ", idx);
            }
        }
        printf("\n");
    }

    // Second: Show block access pattern
    printf("\nBlock access pattern:\n");
    printf("Iteration sequence in the block:\n");
    int iteration = 1;
    for (int dy = 0; dy < subsample_factor && (start_y + dy) < height; dy++)
    {
        for (int dx = 0; dx < subsample_factor && (start_x + dx) < width; dx++)
        {
            int y = start_y + dy;
            int x = start_x + dx;
            int idx = y * width + x;
            printf("\nIteration %d:\n", iteration++);
            printf("  dy=%d, dx=%d\n", dy, dx);
            printf("  2D position: (x=%d, y=%d)\n", x, y);
            printf("  1D array index: %d\n", idx);
        }
    }

    // Third: Show the mathematical breakdown
    printf("\nIndex calculation breakdown for each position:\n");
    for (int dy = 0; dy < subsample_factor && (start_y + dy) < height; dy++)
    {
        for (int dx = 0; dx < subsample_factor && (start_x + dx) < width; dx++)
        {
            int y = start_y + dy;
            int x = start_x + dx;
            printf("\nPosition (%d,%d):\n", x, y);
            printf("  idx = (y + dy) * width + (x + dx)\n");
            printf("  idx = (%d + %d) * %d + (%d + %d)\n",
                   start_y, dy, width, start_x, dx);
            printf("  idx = %d * %d + %d\n", y, width, x);
            printf("  idx = %d + %d = %d\n",
                   y * width, x, y * width + x);
        }
    }
}

int main()
{
    // Example with a 6x4 image and 2x2 block starting at (1,1)
    visualize_block_access(6, 4, 1, 1, 2);
    return 0;
}