// dct.c

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// We're using a magic number called PI (like a circle!)
#define PI 3.14159265358979323846
// We work with small 8x8 squares of the picture, like a tiny puzzle piece
#define BLOCK_SIZE 8

// Think of this as a special box where we keep numbers that describe our picture
typedef struct
{
    double data[BLOCK_SIZE][BLOCK_SIZE];
} DctBlock;

// This is like a recipe card that tells us how much detail to keep in our picture
// Bigger numbers mean we'll throw away more detail to make the file smaller!
const int quantization_matrix_Y[8][8] = {
    {16, 11, 10, 16, 24, 40, 51, 61},
    {12, 12, 14, 19, 26, 58, 60, 55},
    {14, 13, 16, 24, 40, 57, 69, 56},
    {14, 17, 22, 29, 51, 87, 80, 62},
    {18, 22, 37, 56, 68, 109, 103, 77},
    {24, 35, 55, 64, 81, 104, 113, 92},
    {49, 64, 78, 87, 103, 121, 120, 101},
    {72, 92, 95, 98, 112, 100, 103, 99}};

// This function is like magic! It takes our normal picture and turns it into special numbers
// Imagine taking a rainbow and splitting it into different colors
DctBlock apply_dct(unsigned char input[BLOCK_SIZE][BLOCK_SIZE])
{
    DctBlock dct = {0}; // Start with an empty box

    // We're going to look at every tiny spot in our 8x8 square
    for (int u = 0; u < BLOCK_SIZE; u++)
    {
        for (int v = 0; v < BLOCK_SIZE; v++)
        {
            // Some special math magic numbers
            double cu = (u == 0) ? 1.0 / sqrt(2) : 1.0;
            double cv = (v == 0) ? 1.0 / sqrt(2) : 1.0;
            double sum = 0.0;

            // Now we look at each pixel (tiny dot) in our picture
            for (int x = 0; x < BLOCK_SIZE; x++)
            {
                for (int y = 0; y < BLOCK_SIZE; y++)
                {
                    // Make the numbers easier to work with (like counting from zero)
                    double pixel = input[x][y] - 128.0;

                    // More magic math that helps us find patterns in the picture
                    double cos1 = cos((2.0 * x + 1.0) * u * PI / 16.0);
                    double cos2 = cos((2.0 * y + 1.0) * v * PI / 16.0);
                    sum += pixel * cos1 * cos2;
                }
            }

            // Save our special pattern number
            dct.data[u][v] = 0.25 * cu * cv * sum;
        }
    }
    return dct;
}

// This function is like being a neat person - we round numbers to make them simpler
// Just like saying "I'm almost 6 years old" instead of "I'm 5 years, 11 months, and 25 days old"
void quantize_block(DctBlock *dct, const int quantization[BLOCK_SIZE][BLOCK_SIZE])
{
    for (int u = 0; u < BLOCK_SIZE; u++)
    {
        for (int v = 0; v < BLOCK_SIZE; v++)
        {
            // Divide by our recipe card numbers and round to make things simpler
            dct->data[u][v] = round(dct->data[u][v] / quantization[u][v]);
        }
    }
}

// This is like following a special zigzag path through our numbers
// Imagine a bee flying in a zigzag pattern to collect all the numbers
const int ZIGZAG_PATTERN[64][2] = {
    {0, 0}, {0, 1}, {1, 0}, {2, 0}, {1, 1}, {0, 2}, {0, 3}, {1, 2}, {2, 1}, {3, 0}, {4, 0}, {3, 1}, {2, 2}, {1, 3}, {0, 4}, {0, 5}, {1, 4}, {2, 3}, {3, 2}, {4, 1}, {5, 0}, {6, 0}, {5, 1}, {4, 2}, {3, 3}, {2, 4}, {1, 5}, {0, 6}, {0, 7}, {1, 6}, {2, 5}, {3, 4}, {4, 3}, {5, 2}, {6, 1}, {7, 0}, {7, 1}, {6, 2}, {5, 3}, {4, 4}, {3, 5}, {2, 6}, {1, 7}, {2, 7}, {3, 6}, {4, 5}, {5, 4}, {6, 3}, {7, 2}, {7, 3}, {6, 4}, {5, 5}, {4, 6}, {3, 7}, {4, 7}, {5, 6}, {6, 5}, {7, 4}, {7, 5}, {6, 6}, {5, 7}, {6, 7}, {7, 6}, {7, 7}};

// This function follows our zigzag path and collects all the numbers in order
void zigzag_scan(const DctBlock *dct, int output[BLOCK_SIZE * BLOCK_SIZE])
{
    // Follow the zigzag path like a treasure map!
    for (int i = 0; i < BLOCK_SIZE * BLOCK_SIZE; i++)
    {
        int row = ZIGZAG_PATTERN[i][0];
        int col = ZIGZAG_PATTERN[i][1];
        output[i] = (int)dct->data[row][col];
    }
}

// This is like counting how many zeros we have in a row
// Instead of saying "zero, zero, zero, five", we say "three zeros, then five!"
typedef struct
{
    int value;      // The special number we found
    int run_length; // How many zeros came before it
} RLECode;

// This function counts zeros and remembers the important numbers
int run_length_encode(const int zigzag[BLOCK_SIZE * BLOCK_SIZE],
                      RLECode output[BLOCK_SIZE * BLOCK_SIZE])
{
    int code_count = 0;
    int zero_count = 0;

    // Look at each number in our zigzag path
    for (int i = 0; i < BLOCK_SIZE * BLOCK_SIZE; i++)
    {
        if (zigzag[i] == 0)
        {
            // Count another zero!
            zero_count++;
        }
        else
        {
            // Found a non-zero number! Remember how many zeros came before it
            output[code_count].run_length = zero_count;
            output[code_count].value = zigzag[i];
            code_count++;
            zero_count = 0; // Start counting zeros again
        }
    }

    // If we ended with zeros, we need to remember that too
    if (zero_count > 0)
    {
        output[code_count].run_length = 0;
        output[code_count].value = 0;
        code_count++;
    }

    return code_count;
}

// This function shows us what our picture looks like at each step
void process_image_block(unsigned char input[BLOCK_SIZE][BLOCK_SIZE])
{
    printf("Let's compress a tiny piece of our picture!\n\n");

    printf("Step 1: Here's our original tiny square of the picture:\n");
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        for (int j = 0; j < BLOCK_SIZE; j++)
        {
            printf("%3d ", input[i][j]);
        }
        printf("\n");
    }

    // Step 1: Turn our picture into special pattern numbers
    DctBlock dct = apply_dct(input);

    // Step 2: Make the numbers simpler
    quantize_block(&dct, quantization_matrix_Y);

    printf("\nStep 2: Here's our picture after we found patterns in it:\n");
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        for (int j = 0; j < BLOCK_SIZE; j++)
        {
            printf("%3.0f ", dct.data[i][j]);
        }
        printf("\n");
    }

    // Step 3: Collect numbers in a zigzag pattern
    int zigzag_data[BLOCK_SIZE * BLOCK_SIZE];
    zigzag_scan(&dct, zigzag_data);

    // Step 4: Count the zeros to make our file smaller
    RLECode rle_codes[BLOCK_SIZE * BLOCK_SIZE];
    int code_count = run_length_encode(zigzag_data, rle_codes);

    printf("\nStep 3: After counting zeros, here's how we remember our picture:\n");
    for (int i = 0; i < code_count; i++)
    {
        printf("(%d zeros, then %d) ", rle_codes[i].run_length, rle_codes[i].value);
    }
    printf("\n\nAnd that's how we made our picture smaller! 🎉\n");
}

// This function makes a sample picture for us to try
void generate_sample_block(unsigned char block[BLOCK_SIZE][BLOCK_SIZE])
{
    // Make a simple pattern that goes from dark to light
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        for (int j = 0; j < BLOCK_SIZE; j++)
        {
            block[i][j] = (unsigned char)((i + j) * 16);
        }
    }
}

int main()
{
    printf("Let's Play with Picture Compression! 🖼️\n");
    printf("======================================\n\n");

    // Make a sample picture to play with
    unsigned char sample_block[BLOCK_SIZE][BLOCK_SIZE];
    generate_sample_block(sample_block);

    // Try compressing our sample picture
    process_image_block(sample_block);

    // Let's try with your own numbers!
    printf("\nWould you like to try with your own numbers? (y/n): ");
    char response;
    scanf(" %c", &response);

    if (response == 'y' || response == 'Y')
    {
        printf("\nGive me 64 numbers between 0 and 255 (that's like the brightness of each tiny dot):\n");
        unsigned char custom_block[BLOCK_SIZE][BLOCK_SIZE];

        for (int i = 0; i < BLOCK_SIZE; i++)
        {
            for (int j = 0; j < BLOCK_SIZE; j++)
            {
                int value;
                scanf("%d", &value);
                custom_block[i][j] = (unsigned char)(value % 256);
            }
        }

        printf("\nLet's see what happens with your numbers!\n");
        process_image_block(custom_block);
    }

    return 0;
}