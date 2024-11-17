#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    unsigned char r, g, b;
} RGB;

typedef struct
{
    unsigned char y, cb, cr;
} YCbCr;

YCbCr rgb_to_ycbcr(RGB rgb)
{
    YCbCr ycbcr;

    ycbcr.y = (unsigned char)(0.299 * rgb.r + 0.587 * rgb.g + 0.114 * rgb.b);
    ycbcr.cb = (unsigned char)(-0.169 * rgb.r - 0.331 * rgb.g + 0.500 * rgb.b + 128);
    ycbcr.cr = (unsigned char)(0.500 * rgb.r - 0.419 * rgb.g - 0.081 * rgb.b + 128);

    return ycbcr;
}

RGB ycbcr_to_rgb(YCbCr ycbcr)
{
    RGB rgb;
    int r, g, b;

    r = ycbcr.y + 1.402 * (ycbcr.cr - 128);
    g = ycbcr.y - 0.344 * (ycbcr.cb - 128) - 0.714 * (ycbcr.cr - 128);
    b = ycbcr.y + 1.772 * (ycbcr.cb - 128);

    rgb.r = (unsigned char)(r < 0 ? 0 : (r > 255 ? 255 : r));
    rgb.g = (unsigned char)(g < 0 ? 0 : (g > 255 ? 255 : g));
    rgb.b = (unsigned char)(b < 0 ? 0 : (b > 255 ? 255 : b));

    return rgb;
}

void print_subsampling_pattern(YCbCr *data, int width, int height, int x, int y, int factor)
{
    printf("\nSubsampling block at position (%d,%d):\n", x, y);
    for (int dy = 0; dy < factor && (y + dy) < height; dy++)
    {
        for (int dx = 0; dx < factor && (x + dx) < width; dx++)
        {
            int idx = (y + dy) * width + (x + dx);
            printf("Cb:%3d Cr:%3d | ", data[idx].cb, data[idx].cr);
        }
        printf("\n");
    }
}

void subsample_ycbcr_detailed(YCbCr *input, int width, int height, int subsample_factor)
{
    printf("Starting subsampling with factor %dx%d\n", subsample_factor, subsample_factor);

    // Process the image in blocks of subsample_factor x subsample_factor
    for (int y = 0; y < height; y += subsample_factor)
    {
        for (int x = 0; x < width; x += subsample_factor)
        {
            // Print original values in this block
            printf("\nProcessing block at (%d,%d)\n", x, y);
            print_subsampling_pattern(input, width, height, x, y, subsample_factor);

            // Step 1: Calculate averages for Cb and Cr in the block
            int sum_cb = 0, sum_cr = 0;
            int count = 0;

            // Accumulate values within the block
            // Step 1: Start at block origin (x,y)
            // Step 2: Use dy,dx to move within block
            // Step 3: Convert current position to 1D index
            // Step 4: Access data at that index
            // Note: The input array is stored as 1D in memory
            // We're using 2D logic (nested loops with dy,dx) to process a conceptual 2D block
            // The formula converts our 2D block position back to the correct 1D array index
            // This allows us to think in 2D while working with 1D memory
            for (int dy = 0; dy < subsample_factor && (y + dy) < height; dy++)
            {
                for (int dx = 0; dx < subsample_factor && (x + dx) < width; dx++)
                {
                    int idx = (y + dy) * width + (x + dx);
                    sum_cb += input[idx].cb;
                    sum_cr += input[idx].cr;
                    count++;
                }
            }

            // Step 2: Calculate average values
            unsigned char avg_cb = sum_cb / count;
            unsigned char avg_cr = sum_cr / count;

            printf("Block averages - Cb: %d, Cr: %d (from %d pixels)\n",
                   avg_cb, avg_cr, count);

            // Step 3: Apply averaged values back to all pixels in the block
            for (int dy = 0; dy < subsample_factor && (y + dy) < height; dy++)
            {
                for (int dx = 0; dx < subsample_factor && (x + dx) < width; dx++)
                {
                    int idx = (y + dy) * width + (x + dx);
                    input[idx].cb = avg_cb;
                    input[idx].cr = avg_cr;
                }
            }

            // Print subsampled values
            printf("\nAfter subsampling:\n");
            print_subsampling_pattern(input, width, height, x, y, subsample_factor);
        }
    }
}

int main()
{
    const int WIDTH = 8;
    const int HEIGHT = 8;
    const int SUBSAMPLE_FACTOR = 2; // 4:2:0 subsampling

    // Create test image data
    YCbCr *image = malloc(WIDTH * HEIGHT * sizeof(YCbCr));

    // Initialize with test values
    for (int i = 0; i < WIDTH * HEIGHT; i++)
    {
        image[i].y = 128;                  // Mid-gray luminance
        image[i].cb = 100 + (i % 256) / 4; // Varying Cb values
        image[i].cr = 150 + (i % 256) / 4; // Varying Cr values
    }

    // Perform subsampling
    subsample_ycbcr_detailed(image, WIDTH, HEIGHT, SUBSAMPLE_FACTOR);

    free(image);
    return 0;
}