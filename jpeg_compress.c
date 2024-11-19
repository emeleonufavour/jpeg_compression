#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <jpeglib.h>
#include "jpeg_common.h"

static const uint8_t STD_QUANT_TABLE_Y[BLOCK_SIZE][BLOCK_SIZE] = {
    {16, 11, 10, 16, 24, 40, 51, 61},
    {12, 12, 14, 19, 26, 58, 60, 55},
    {14, 13, 16, 24, 40, 57, 69, 56},
    {14, 17, 22, 29, 51, 87, 80, 62},
    {18, 22, 37, 56, 68, 109, 103, 77},
    {24, 35, 55, 64, 81, 104, 113, 92},
    {49, 64, 78, 87, 103, 121, 120, 101},
    {72, 92, 95, 98, 112, 100, 103, 99}};

static const uint8_t STD_QUANT_TABLE_C[BLOCK_SIZE][BLOCK_SIZE] = {
    {17, 18, 24, 47, 99, 99, 99, 99},
    {18, 21, 26, 66, 99, 99, 99, 99},
    {24, 26, 56, 99, 99, 99, 99, 99},
    {47, 66, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99}};

// Add Huffman tables (standard JPEG tables)
static const uint8_t STD_DC_LUMINANCE_CODES[] = {0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0}; // BITS
static const uint8_t STD_DC_LUMINANCE_VALUES[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

static const uint8_t STD_AC_LUMINANCE_CODES[] = {0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 125};
static const uint8_t STD_AC_LUMINANCE_VALUES[] = {
    0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
    0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
    0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
    0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
    0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
    0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
    0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
    0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
    0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
    0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
    0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
    0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5};

int init_huffman_tables(JpegState *state)
{
    // Standard DC and AC Huffman table values
    static const uint8_t DC_LUMINANCE_BITS[] = {0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0}; // 12 values
    static const uint8_t DC_LUMINANCE_VALUES[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

    static const uint8_t AC_LUMINANCE_BITS[] = {0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 125}; // 16 values
    static const uint8_t AC_LUMINANCE_VALUES[] = {
        0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
        0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
        0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
        0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
        0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
        0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
        0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
        0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
        0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
        0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
        0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
        0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
        0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
        0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
        0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
        0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5};

    // Standard DC and AC chrominance tables (similar structure)
    static const uint8_t DC_CHROMINANCE_BITS[] = {0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0};
    static const uint8_t DC_CHROMINANCE_VALUES[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

    static const uint8_t AC_CHROMINANCE_BITS[] = {0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 119};
    static const uint8_t AC_CHROMINANCE_VALUES[] = {
        0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
        0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
        0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
        0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
        0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
        0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
        0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
        0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
        0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
        0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
        0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
        0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
        0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
        0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
        0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
        0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3};

    // Allocate memory for Huffman tables
    state->dc_table_y.codes = malloc(sizeof(HuffmanCode) * 12);  // DC luminance needs 12 codes
    state->ac_table_y.codes = malloc(sizeof(HuffmanCode) * 256); // AC luminance needs up to 256 codes
    state->dc_table_c.codes = malloc(sizeof(HuffmanCode) * 12);  // DC chrominance needs 12 codes
    state->ac_table_c.codes = malloc(sizeof(HuffmanCode) * 256); // AC chrominance needs up to 256 codes

    if (!state->dc_table_y.codes || !state->ac_table_y.codes ||
        !state->dc_table_c.codes || !state->ac_table_c.codes)
    {
        return -1; // Memory allocation failed
    }

    // Initialize DC luminance table
    uint16_t code = 0;
    int j = 0;
    for (int i = 0; i < 12; i++)
    {
        for (int k = 0; k < DC_LUMINANCE_BITS[i]; k++)
        {
            state->dc_table_y.codes[j].code_length = i + 1;
            state->dc_table_y.codes[j].code = code;
            code++;
            j++;
        }
        code <<= 1;
    }
    state->dc_table_y.count = j;

    // Initialize AC luminance table
    code = 0;
    j = 0;
    for (int i = 0; i < 16; i++)
    {
        for (int k = 0; k < AC_LUMINANCE_BITS[i]; k++)
        {
            state->ac_table_y.codes[j].code_length = i + 1;
            state->ac_table_y.codes[j].code = code;
            code++;
            j++;
        }
        code <<= 1;
    }
    state->ac_table_y.count = j;

    // Initialize DC chrominance table
    code = 0;
    j = 0;
    for (int i = 0; i < 12; i++)
    {
        for (int k = 0; k < DC_CHROMINANCE_BITS[i]; k++)
        {
            state->dc_table_c.codes[j].code_length = i + 1;
            state->dc_table_c.codes[j].code = code;
            code++;
            j++;
        }
        code <<= 1;
    }
    state->dc_table_c.count = j;

    // Initialize AC chrominance table
    code = 0;
    j = 0;
    for (int i = 0; i < 16; i++)
    {
        for (int k = 0; k < AC_CHROMINANCE_BITS[i]; k++)
        {
            state->ac_table_c.codes[j].code_length = i + 1;
            state->ac_table_c.codes[j].code = code;
            code++;
            j++;
        }
        code <<= 1;
    }
    state->ac_table_c.count = j;

    return 0;
}

// Enhanced buffer management
static void ensure_buffer_capacity(JpegState *state, size_t needed_size)
{
    if (state->buffer_position + needed_size > state->buffer_size)
    {
        size_t new_size = state->buffer_size * 2;
        while (new_size < state->buffer_position + needed_size)
        {
            new_size *= 2;
        }

        uint8_t *new_buffer = realloc(state->output_buffer, new_size);
        if (!new_buffer)
        {
            // Handle allocation failure
            return;
        }

        state->output_buffer = new_buffer;
        state->buffer_size = new_size;
    }
}

static void write_byte(JpegState *state, uint8_t byte)
{
    if (state->buffer_position >= state->buffer_size)
    {
        size_t new_size = state->buffer_size * 2;
        uint8_t *new_buffer = realloc(state->output_buffer, new_size);
        if (!new_buffer)
            return;
        state->output_buffer = new_buffer;
        state->buffer_size = new_size;
    }
    state->output_buffer[state->buffer_position++] = byte;
}

// Utility functions for writing to output buffer
static void write_marker(JpegState *state, JpegMarker marker)
{
    write_byte(state, 0xFF);
    write_byte(state, marker & 0xFF);
}
static void write_word(JpegState *state, uint16_t word)
{
    write_byte(state, (word >> 8) & 0xFF);
    write_byte(state, word & 0xFF);
}
static void write_app0(JpegState *state)
{
    write_marker(state, 0xE0); // APP0 marker
    write_word(state, 16);     // Length: 16 bytes including length field
    write_byte(state, 'J');    // "JFIF" identifier
    write_byte(state, 'F');
    write_byte(state, 'I');
    write_byte(state, 'F');
    write_byte(state, 0x00); // Null terminator
    write_byte(state, 1);    // Major version
    write_byte(state, 2);    // Minor version
    write_byte(state, 0);    // Density units (0 = no units, aspect ratio only)
    write_word(state, 1);    // X density (1 pixel per unit)
    write_word(state, 1);    // Y density (1 pixel per unit)
    write_byte(state, 0);    // Thumbnail width
    write_byte(state, 0);    // Thumbnail height
}

static void write_dht(JpegState *state)
{
    // Start of DHT marker
    write_marker(state, 0xC4);

    // Compute length of DHT segment
    size_t length = 2; // Length field itself
    length += 1 + 12;  // DC Luminance table
    length += 1 + 162; // AC Luminance table
    length += 1 + 12;  // DC Chrominance table
    length += 1 + 162; // AC Chrominance table

    write_word(state, length);

    // Write DC Luminance table
    write_byte(state, 0x00); // Table ID 0 (DC, Luminance)
    for (int i = 0; i < 12; i++)
    {
        write_byte(state, STD_DC_LUMINANCE_CODES[i]); // BITS
    }
    for (int i = 0; i < 12; i++)
    {
        write_byte(state, STD_DC_LUMINANCE_VALUES[i]); // VALUES
    }

    // Write AC Luminance table
    write_byte(state, 0x10); // Table ID 1 (AC, Luminance)
    for (int i = 0; i < 16; i++)
    {
        write_byte(state, STD_AC_LUMINANCE_CODES[i]); // BITS
    }
    for (int i = 0; i < 162; i++)
    {
        write_byte(state, STD_AC_LUMINANCE_VALUES[i]); // VALUES
    }

    // Write DC Chrominance table
    write_byte(state, 0x01); // Table ID 0 (DC, Chrominance)
    for (int i = 0; i < 12; i++)
    {
        write_byte(state, STD_DC_LUMINANCE_CODES[i]); // BITS
    }
    for (int i = 0; i < 12; i++)
    {
        write_byte(state, STD_DC_LUMINANCE_VALUES[i]); // VALUES
    }

    // Write AC Chrominance table
    write_byte(state, 0x11); // Table ID 1 (AC, Chrominance)
    for (int i = 0; i < 16; i++)
    {
        write_byte(state, STD_AC_LUMINANCE_CODES[i]); // BITS
    }
    for (int i = 0; i < 162; i++)
    {
        write_byte(state, STD_AC_LUMINANCE_VALUES[i]); // VALUES
    }
}

static void write_sos(JpegState *state)
{
    write_marker(state, 0xDA); // Start of Scan marker
    write_word(state, 12);     // Length: 12 bytes

    write_byte(state, 3); // Number of components

    // Component 1: Y
    write_byte(state, 1);    // Component ID
    write_byte(state, 0x00); // Huffman table (DC: 0, AC: 0)

    // Component 2: Cb
    write_byte(state, 2);    // Component ID
    write_byte(state, 0x11); // Huffman table (DC: 1, AC: 1)

    // Component 3: Cr
    write_byte(state, 3);    // Component ID
    write_byte(state, 0x11); // Huffman table (DC: 1, AC: 1)

    // Spectral selection (default for baseline JPEG)
    write_byte(state, 0);  // Start of spectral selection
    write_byte(state, 63); // End of spectral selection
    write_byte(state, 0);  // Successive approximation
}

// Enhanced bit writing with overflow protection
void write_bits(JpegState *state, uint32_t bits, int bit_count)
{
    if (bit_count <= 0 || bit_count > 32)
        return;

    ensure_buffer_capacity(state, (bit_count + state->bits_in_buffer + 7) / 8 + 1);

    while (bit_count > 0)
    {
        // Fill bit buffer from left to right
        state->bit_buffer = (state->bit_buffer << 1) |
                            ((bits >> (bit_count - 1)) & 1);
        state->bits_in_buffer++;
        bit_count--;

        if (state->bits_in_buffer == 8)
        {
            write_byte(state, state->bit_buffer);
            if (state->bit_buffer == 0xFF)
            {
                write_byte(state, 0x00); // Byte stuffing
            }
            state->bit_buffer = 0;
            state->bits_in_buffer = 0;
        }
    }
}

static void build_huffman_tables(JpegState *state)
{
    // Allocate memory for Huffman tables
    state->dc_table_y.codes = malloc(sizeof(HuffmanCode) * 12);
    state->ac_table_y.codes = malloc(sizeof(HuffmanCode) * 256);
    state->dc_table_c.codes = malloc(sizeof(HuffmanCode) * 12);
    state->ac_table_c.codes = malloc(sizeof(HuffmanCode) * 256);

    uint16_t code = 0;
    int si;

    // Build DC luminance table
    for (int i = 0, j = 0; i < 16; i++)
    {
        for (si = 0; si < STD_DC_LUMINANCE_CODES[i]; si++)
        {
            state->dc_table_y.codes[j].code_length = i + 1;
            state->dc_table_y.codes[j].code = code;
            j++;
            code++;
        }
        code <<= 1;
    }
    state->dc_table_y.count = 12;

    // Build AC luminance table
    code = 0;
    for (int i = 0, j = 0; i < 16; i++)
    {
        for (si = 0; si < STD_AC_LUMINANCE_CODES[i]; si++)
        {
            state->ac_table_y.codes[j].code_length = i + 1;
            state->ac_table_y.codes[j].code = code;
            j++;
            code++;
        }
        code <<= 1;
    }
    state->ac_table_y.count = 162;

    // Build DC/AC chrominance tables (similar process)
    // ... Similar process for chrominance tables ...
}

static void huffman_encode_block(JpegState *state, const RLECode *codes, size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        if (i == 0)
        {
            // Encode DC coefficient
            int diff = codes[i].value - state->last_dc_y;
            state->last_dc_y = codes[i].value;

            // Calculate size category
            int size = 0;
            int abs_diff = abs(diff);
            while (abs_diff > 0)
            {
                size++;
                abs_diff >>= 1;
            }

            // Write size using DC Huffman table
            const HuffmanCode *huff_code = &state->dc_table_y.codes[size];
            write_bits(state, huff_code->code, huff_code->code_length);

            // Write amplitude
            if (diff < 0)
            {
                diff = diff - 1;
            }
            if (size > 0)
            {
                write_bits(state, diff & ((1 << size) - 1), size);
            }
        }
        else
        {
            // Encode AC coefficient
            int run_length = codes[i].run_length;
            int value = codes[i].value;

            // Calculate size category
            int size = 0;
            int abs_value = abs(value);
            while (abs_value > 0)
            {
                size++;
                abs_value >>= 1;
            }

            // Create composite value for Huffman lookup
            int composite = (run_length << 4) | size;

            // Write using AC Huffman table
            const HuffmanCode *huff_code = &state->ac_table_y.codes[composite];
            write_bits(state, huff_code->code, huff_code->code_length);

            // Write amplitude
            if (value < 0)
            {
                value = value - 1;
            }
            if (size > 0)
            {
                write_bits(state, value & ((1 << size) - 1), size);
            }
        }
    }
}

// This function is like magic! It takes our normal picture and turns it into special numbers
// Imagine taking a rainbow and splitting it into different colors
static DctBlock apply_dct(const uint8_t input[BLOCK_SIZE][BLOCK_SIZE])
{
    DctBlock dct = {0};

    for (int u = 0; u < BLOCK_SIZE; u++)
    {
        for (int v = 0; v < BLOCK_SIZE; v++)
        {
            const double cu = (u == 0) ? 1.0 / sqrt(2) : 1.0;
            const double cv = (v == 0) ? 1.0 / sqrt(2) : 1.0;
            double sum = 0.0;

            for (int x = 0; x < BLOCK_SIZE; x++)
            {
                for (int y = 0; y < BLOCK_SIZE; y++)
                {
                    const double pixel = input[x][y] - 128.0;
                    const double cos1 = cos((2.0 * x + 1.0) * u * PI / 16.0);
                    const double cos2 = cos((2.0 * y + 1.0) * v * PI / 16.0);
                    sum += pixel * cos1 * cos2;
                }
            }

            dct.data[u][v] = 0.25 * cu * cv * sum;
        }
    }

    return dct;
}

// This function is like being a neat person - we round numbers to make them simpler
// Just like saying "I'm almost 6 years old" instead of "I'm 5 years, 11 months, and 25 days old"
static void quantize_block(DctBlock *dct, const uint8_t quant_table[BLOCK_SIZE][BLOCK_SIZE])
{
    for (int u = 0; u < BLOCK_SIZE; u++)
    {
        for (int v = 0; v < BLOCK_SIZE; v++)
        {
            dct->data[u][v] = round(dct->data[u][v] / quant_table[u][v]);
        }
    }
}

const int ZIGZAG_PATTERN[64][2] = {
    {0, 0}, {0, 1}, {1, 0}, {2, 0}, {1, 1}, {0, 2}, {0, 3}, {1, 2}, {2, 1}, {3, 0}, {4, 0}, {3, 1}, {2, 2}, {1, 3}, {0, 4}, {0, 5}, {1, 4}, {2, 3}, {3, 2}, {4, 1}, {5, 0}, {6, 0}, {5, 1}, {4, 2}, {3, 3}, {2, 4}, {1, 5}, {0, 6}, {0, 7}, {1, 6}, {2, 5}, {3, 4}, {4, 3}, {5, 2}, {6, 1}, {7, 0}, {7, 1}, {6, 2}, {5, 3}, {4, 4}, {3, 5}, {2, 6}, {1, 7}, {2, 7}, {3, 6}, {4, 5}, {5, 4}, {6, 3}, {7, 2}, {7, 3}, {6, 4}, {5, 5}, {4, 6}, {3, 7}, {4, 7}, {5, 6}, {6, 5}, {7, 4}, {7, 5}, {6, 6}, {5, 7}, {6, 7}, {7, 6}, {7, 7}};

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

// Enhanced compression pipeline
static void process_mcu(JpegState *state, uint32_t x, uint32_t y)
{
    uint8_t block[BLOCK_SIZE][BLOCK_SIZE];

    // Extract Y (luminance) block
    for (int by = 0; by < BLOCK_SIZE; by++)
    {
        for (int bx = 0; bx < BLOCK_SIZE; bx++)
        {
            if (y + by < state->height && x + bx < state->width)
            {
                block[by][bx] = state->ycbcr_data[(y + by) * state->width + (x + bx)].y;
            }
            else
            {
                block[by][bx] = block[by - 1][bx - 1]; // Edge padding
            }
        }
    }

    // Process Y block
    DctBlock dct = apply_dct(block);
    quantize_block(&dct, STD_QUANT_TABLE_Y);

    // Zigzag scan
    int zigzag_data[BLOCK_SIZE * BLOCK_SIZE];
    zigzag_scan(&dct, zigzag_data);

    // Run-length encode
    RLECode rle_codes[BLOCK_SIZE * BLOCK_SIZE];
    int code_count = run_length_encode(zigzag_data, rle_codes);

    // Huffman encode
    huffman_encode_block(state, rle_codes, code_count);

    // Complete MCU processing for Cb and Cr blocks
    if ((x % (BLOCK_SIZE * state->subsample_factor) == 0) &&
        (y % (BLOCK_SIZE * state->subsample_factor) == 0))
    {
        // Process Cb block
        uint8_t cb_block[BLOCK_SIZE][BLOCK_SIZE];
        for (int by = 0; by < BLOCK_SIZE; by++)
        {
            for (int bx = 0; bx < BLOCK_SIZE; bx++)
            {
                int src_x = x / state->subsample_factor + bx;
                int src_y = y / state->subsample_factor + by;
                if (src_y < state->height && src_x < state->width)
                {
                    cb_block[by][bx] = state->ycbcr_data[src_y * state->width + src_x].cb;
                }
                else
                {
                    cb_block[by][bx] = cb_block[by - 1][bx - 1];
                }
            }
        }

        DctBlock cb_dct = apply_dct(cb_block);
        quantize_block(&cb_dct, STD_QUANT_TABLE_C);

        int cb_zigzag[BLOCK_SIZE * BLOCK_SIZE];
        zigzag_scan(&cb_dct, cb_zigzag);

        RLECode cb_codes[BLOCK_SIZE * BLOCK_SIZE];
        int cb_code_count = run_length_encode(cb_zigzag, cb_codes);

        huffman_encode_block(state, cb_codes, cb_code_count);

        // Process Cr block
        uint8_t cr_block[BLOCK_SIZE][BLOCK_SIZE];
        for (int by = 0; by < BLOCK_SIZE; by++)
        {
            for (int bx = 0; bx < BLOCK_SIZE; bx++)
            {
                int src_x = x / state->subsample_factor + bx;
                int src_y = y / state->subsample_factor + by;
                if (src_y < state->height && src_x < state->width)
                {
                    cr_block[by][bx] = state->ycbcr_data[src_y * state->width + src_x].cr;
                }
                else
                {
                    cr_block[by][bx] = cr_block[by - 1][bx - 1];
                }
            }
        }

        DctBlock cr_dct = apply_dct(cr_block);
        quantize_block(&cr_dct, STD_QUANT_TABLE_C);

        int cr_zigzag[BLOCK_SIZE * BLOCK_SIZE];
        zigzag_scan(&cr_dct, cr_zigzag);

        RLECode cr_codes[BLOCK_SIZE * BLOCK_SIZE];
        int cr_code_count = run_length_encode(cr_zigzag, cr_codes);

        huffman_encode_block(state, cr_codes, cr_code_count);
    }
}

static YCbCr convert_rgb_to_ycbcr(RGB rgb)
{
    YCbCr ycbcr;

    // Using BT.601 conversion
    ycbcr.y = (uint8_t)(0.299 * rgb.r + 0.587 * rgb.g + 0.114 * rgb.b);
    ycbcr.cb = (uint8_t)(-0.169 * rgb.r - 0.331 * rgb.g + 0.500 * rgb.b + 128);
    ycbcr.cr = (uint8_t)(0.500 * rgb.r - 0.419 * rgb.g - 0.081 * rgb.b + 128);

    return ycbcr;
}

static void apply_chroma_subsampling(JpegState *state)
{
    const int factor = state->subsample_factor;

    for (uint32_t y = 0; y < state->height; y += factor)
    {
        for (uint32_t x = 0; x < state->width; x += factor)
        {
            // Calculate average Cb and Cr for the block
            int sum_cb = 0, sum_cr = 0, count = 0;

            for (int dy = 0; dy < factor && (y + dy) < state->height; dy++)
            {
                for (int dx = 0; dx < factor && (x + dx) < state->width; dx++)
                {
                    const uint32_t idx = (y + dy) * state->width + (x + dx);
                    sum_cb += state->ycbcr_data[idx].cb;
                    sum_cr += state->ycbcr_data[idx].cr;
                    count++;
                }
            }

            // Apply averages back to the block
            const uint8_t avg_cb = sum_cb / count;
            const uint8_t avg_cr = sum_cr / count;

            for (int dy = 0; dy < factor && (y + dy) < state->height; dy++)
            {
                for (int dx = 0; dx < factor && (x + dx) < state->width; dx++)
                {
                    const uint32_t idx = (y + dy) * state->width + (x + dx);
                    state->ycbcr_data[idx].cb = avg_cb;
                    state->ycbcr_data[idx].cr = avg_cr;
                }
            }
        }
    }
}

// This function follows our zigzag path and collects all the numbers in order

// Initialize quantization tables with quality scaling
static void init_quantization_tables(JpegState *state)
{
    double quality_scale;
    if (state->quality < 50)
        quality_scale = 50.0 / state->quality;
    else
        quality_scale = 2.0 - ((double)state->quality / 50.0);

    // Scale luminance quantization table
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        for (int j = 0; j < BLOCK_SIZE; j++)
        {
            int value = (int)(STD_QUANT_TABLE_Y[i][j] * quality_scale);
            state->quant_table_y[i * BLOCK_SIZE + j] =
                (uint8_t)CLAMP(value, 1, 255);
        }
    }

    // Scale chrominance quantization table
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        for (int j = 0; j < BLOCK_SIZE; j++)
        {
            int value = (int)(STD_QUANT_TABLE_C[i][j] * quality_scale);
            state->quant_table_c[i * BLOCK_SIZE + j] =
                (uint8_t)CLAMP(value, 1, 255);
        }
    }
}

void jpeg_cleanup(JpegState *state)
{
    if (!state)
        return;

    if (state->output_buffer)
    {
        free(state->output_buffer);
        state->output_buffer = NULL;
    }
    if (state->rgb_data)
    {
        free(state->rgb_data);
        state->rgb_data = NULL;
    }
    if (state->ycbcr_data)
    {
        free(state->ycbcr_data);
        state->ycbcr_data = NULL;
    }
    if (state->quant_table_y)
    {
        free(state->quant_table_y);
        state->quant_table_y = NULL;
    }
    if (state->quant_table_c)
    {
        free(state->quant_table_c);
        state->quant_table_c = NULL;
    }

    if (state->outfile)
    {
        fclose(state->outfile);
        state->outfile = NULL;
    }

    free(state);
}

// Initialize JPEG compression state

// Add proper initialization function
// Input validation and initialization enhancements
JpegState *jpeg_init(uint32_t width, uint32_t height, uint8_t quality)
{
    // Validate input parameters
    if (width == 0 || height == 0 || width > 65535 || height > 65535)
    {
        return NULL;
    }

    // Quality should be between 1 and 100
    if (quality < 1 || quality > 100)
    {
        quality = 75; // Default quality
    }

    JpegState *state = calloc(1, sizeof(JpegState));
    if (!state)
        return NULL;

    state->width = width;
    state->height = height;
    state->quality = quality;
    state->subsample_factor = 2; // 4:2:0 subsampling

    // Calculate buffer sizes with overflow protection
    size_t pixel_count = (size_t)width * height;
    if (pixel_count / width != height)
    { // Check for overflow
        free(state);
        return NULL;
    }

    // Initial buffer size estimation (conservative)
    state->buffer_size = pixel_count * 3; // Roughly 3 bytes per pixel

    // Allocate all required buffers
    state->output_buffer = malloc(width * height * 3);
    if (!state->output_buffer)
        goto cleanup;

    state->rgb_data = malloc(width * height * sizeof(RGB));
    if (!state->rgb_data)
        goto cleanup;

    state->ycbcr_data = malloc(width * height * sizeof(YCbCr));
    if (!state->ycbcr_data)
        goto cleanup;

    state->quant_table_y = malloc(BLOCK_SIZE * BLOCK_SIZE);
    if (!state->quant_table_y)
        goto cleanup;

    state->quant_table_c = malloc(BLOCK_SIZE * BLOCK_SIZE);
    if (!state->quant_table_c)
        goto cleanup;

    if (!state->output_buffer || !state->rgb_data || !state->ycbcr_data ||
        !state->quant_table_y || !state->quant_table_c)
    {
        jpeg_cleanup(state);
        return NULL;
    }

    // Initialize quantization tables with quality scaling
    init_quantization_tables(state);

    // Initialize Huffman tables
    if (init_huffman_tables(state) != 0)
    {
        jpeg_cleanup(state);
        return NULL;
    }

    return state;

cleanup:
    jpeg_cleanup(state);
    return NULL;
}

// Write Start of Frame
void write_sof0(JpegState *state)
{
    write_marker(state, MARKER_SOF0);
    write_word(state, 17); // Length
    write_byte(state, 8);  // Precision
    write_word(state, state->height);
    write_word(state, state->width);
    write_byte(state, 3); // Number of components

    // Y component
    write_byte(state, 1);    // Component ID
    write_byte(state, 0x22); // Sampling factors (2x2)
    write_byte(state, 0);    // Quant table ID

    // Cb component
    write_byte(state, 2);    // Component ID
    write_byte(state, 0x11); // Sampling factors (1x1)
    write_byte(state, 1);    // Quant table ID

    // Cr component
    write_byte(state, 3);    // Component ID
    write_byte(state, 0x11); // Sampling factors (1x1)
    write_byte(state, 1);    // Quant table ID
}

// Write quantization tables
void write_dqt(JpegState *state)
{
    // Write marker and length
    write_marker(state, MARKER_DQT);
    write_word(state, 2 + (2 * (1 + 64))); // Length

    // Write luminance table
    write_byte(state, 0x00); // Table ID 0, precision 8-bit
    for (int i = 0; i < 64; i++)
    {
        write_byte(state, state->quant_table_y[ZIGZAG_PATTERN[i][0] * 8 +
                                               ZIGZAG_PATTERN[i][1]]);
    }

    // Write chrominance table
    write_byte(state, 0x01); // Table ID 1, precision 8-bit
    for (int i = 0; i < 64; i++)
    {
        write_byte(state, state->quant_table_c[ZIGZAG_PATTERN[i][0] * 8 +
                                               ZIGZAG_PATTERN[i][1]]);
    }
}

// Write JPEG file header
void write_jpeg_header(JpegState *state)
{
    // Write SOI marker
    write_marker(state, MARKER_SOI);

    // Write JFIF APP0 marker
    write_app0(state); // TODO: Implement write_app

    // Write quantization tables
    write_dqt(state);

    // Write Start of Frame
    write_sof0(state);

    // Write Huffman tables
    write_dht(state); // TODO: Implement write dht

    // Write Start of Scan
    write_sos(state); // TODO: Implement write_sos
}

// Write JPEG file trailer
void write_jpeg_trailer(JpegState *state)
{
    // Write End of Image marker
    fwrite("\xFF\xD9", 1, 2, state->outfile);
}

// Main compression function
// Main compression function enhancement
int jpeg_compress(JpegState *state, const char *output_filename)
{
    if (!state || !output_filename)
        return -1;

    // Open output file
    state->outfile = fopen(output_filename, "wb");
    if (!state->outfile)
        return -1;

    // Initialize compression state
    state->bit_buffer = 0;
    state->bits_in_buffer = 0;
    state->last_dc_y = 0;
    state->last_dc_cb = 0;
    state->last_dc_cr = 0;

    // Write JPEG headers
    write_jpeg_header(state);

    // Convert colorspace and apply subsampling
    for (uint32_t i = 0; i < state->width * state->height; i++)
    {
        state->ycbcr_data[i] = convert_rgb_to_ycbcr(state->rgb_data[i]);
    }
    apply_chroma_subsampling(state);

    // Process MCUs
    for (uint32_t y = 0; y < state->height; y += BLOCK_SIZE)
    {
        for (uint32_t x = 0; x < state->width; x += BLOCK_SIZE)
        {
            process_mcu(state, x, y);
        }
    }

    // Flush remaining bits
    if (state->bits_in_buffer > 0)
    {
        write_byte(state, state->bit_buffer << (8 - state->bits_in_buffer));
    }

    // Write JPEG trailer
    write_jpeg_trailer(state);

    // Write all buffered data to file
    fwrite(state->output_buffer, 1, state->buffer_position, state->outfile);

    return 0;
}

// Function to decode a JPEG image into an RGB array
RGB *read_jpeg(const char *filename, uint32_t *width, uint32_t *height)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    FILE *infile = fopen(filename, "rb");
    if (!infile)
    {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        return NULL;
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);

    jpeg_start_decompress(&cinfo);
    *width = cinfo.output_width;
    *height = cinfo.output_height;

    size_t row_stride = cinfo.output_width * cinfo.output_components;
    RGB *pixels = malloc((*width) * (*height) * sizeof(RGB));
    if (!pixels)
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return NULL;
    }

    JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);
    for (uint32_t y = 0; y < cinfo.output_height; y++)
    {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        for (uint32_t x = 0; x < cinfo.output_width; x++)
        {
            pixels[y * (*width) + x].r = buffer[0][x * 3 + 0];
            pixels[y * (*width) + x].g = buffer[0][x * 3 + 1];
            pixels[y * (*width) + x].b = buffer[0][x * 3 + 2];
        }
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return pixels;
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <input.jpg> <output.jpg> <quality>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *input_filename = argv[1];
    const char *output_filename = argv[2];
    uint8_t quality = (uint8_t)atoi(argv[3]);

    uint32_t width, height;
    RGB *rgb_data = read_jpeg(input_filename, &width, &height);
    if (!rgb_data)
    {
        return EXIT_FAILURE;
    }

    // Initialize JPEG state
    JpegState *jpeg_state = jpeg_init(width, height, quality);
    if (!jpeg_state)
    {
        fprintf(stderr, "Error: Failed to initialize JPEG state\n");
        free(rgb_data);
        return EXIT_FAILURE;
    }

    jpeg_state->rgb_data = rgb_data;

    // Perform JPEG compression
    if (jpeg_compress(jpeg_state, output_filename) != 0)
    {
        fprintf(stderr, "Error: JPEG compression failed\n");
        jpeg_cleanup(jpeg_state);
        free(rgb_data);
        return EXIT_FAILURE;
    }

    printf("JPEG compression successful: %s\n", output_filename);

    // Clean up
    jpeg_cleanup(jpeg_state);
    free(rgb_data);

    return EXIT_SUCCESS;
}