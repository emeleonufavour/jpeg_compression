// jpeg_common.h
#ifndef JPEG_COMMON_H
#define JPEG_COMMON_H

#define CLAMP(value, min, max) ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))

#include <stdint.h>
#include <stdio.h>

// Constants
#define BLOCK_SIZE 8
#define PI 3.14159265358979323846

// Basic color structures
typedef struct
{
    uint8_t r, g, b;
} RGB;

typedef struct
{
    uint8_t y, cb, cr;
} YCbCr;

// Compression related structures
typedef struct
{
    double data[BLOCK_SIZE][BLOCK_SIZE];
} DctBlock;

typedef struct
{
    int16_t value;      // The value of the coefficient
    uint8_t run_length; // Number of zeros before this coefficient
} RLECode;

// Huffman coding structures
typedef struct
{
    uint8_t code_length; // Length of the Huffman code
    uint16_t code;       // The Huffman code itself
} HuffmanCode;

typedef struct
{
    HuffmanCode *codes; // Array of Huffman codes
    uint16_t count;     // Number of codes
} HuffmanTable;

// JPEG markers
typedef enum
{
    MARKER_SOI = 0xFFD8,  // Start of Image
    MARKER_EOI = 0xFFD9,  // End of Image
    MARKER_SOF0 = 0xFFC0, // Start of Frame (Baseline DCT)
    MARKER_DHT = 0xFFC4,  // Define Huffman Table
    MARKER_DQT = 0xFFDB,  // Define Quantization Table
    MARKER_SOS = 0xFFDA   // Start of Scan
} JpegMarker;

// Complete JPEG state
typedef struct
{
    // Image properties
    uint32_t width;
    uint32_t height;
    uint8_t quality;
    uint8_t subsample_factor;

    // Image data
    RGB *rgb_data;
    YCbCr *ycbcr_data;

    // Output handling
    FILE *outfile;
    uint8_t *output_buffer;
    uint32_t buffer_size;
    uint32_t buffer_position;

    // Bit writing state
    uint8_t bit_buffer;
    uint8_t bits_in_buffer;

    // Quantization tables
    uint8_t *quant_table_y; // Luminance quantization table
    uint8_t *quant_table_c; // Chrominance quantization table

    // Huffman tables
    HuffmanTable dc_table_y; // DC luminance
    HuffmanTable ac_table_y; // AC luminance
    HuffmanTable dc_table_c; // DC chrominance
    HuffmanTable ac_table_c; // AC chrominance

    // DC coefficient tracking
    int16_t last_dc_y;  // Last DC value for Y component
    int16_t last_dc_cb; // Last DC value for Cb component
    int16_t last_dc_cr; // Last DC value for Cr component
} JpegState;

// Standard quantization tables
// extern const uint8_t STD_QUANT_TABLE_Y[BLOCK_SIZE][BLOCK_SIZE];
// extern const uint8_t STD_QUANT_TABLE_C[BLOCK_SIZE][BLOCK_SIZE];

// Core functions
// JpegState *jpeg_init(uint32_t width, uint32_t height, uint8_t quality);
// void jpeg_cleanup(JpegState *state);
// int jpeg_compress(JpegState *state, const char *output_filename);

// // Component functions
// YCbCr rgb_to_ycbcr(RGB pixel);
// void subsample_ycbcr(JpegState *state);
// DctBlock apply_dct(const uint8_t block[BLOCK_SIZE][BLOCK_SIZE]);
// void quantize_block(DctBlock *block, const uint8_t quant_table[BLOCK_SIZE][BLOCK_SIZE]);
// void zigzag_scan(const DctBlock *block, int16_t *output);
// int run_length_encode(const int16_t *data, RLECode *codes, size_t data_len);

// // Huffman coding functions
// void init_huffman_tables(JpegState *state);
// void huffman_encode_block(JpegState *state, const RLECode *codes, size_t code_count,
//                           int is_dc, int is_chroma);

// // Bit writing functions
// void write_bits(JpegState *state, uint32_t bits, int bit_count);
// void flush_bits(JpegState *state);

// // JPEG segment writing functions
// void write_soi(JpegState *state);
// void write_eoi(JpegState *state);
// void write_dqt(JpegState *state);
// void write_sof0(JpegState *state);
// void write_dht(JpegState *state);
// void write_sos(JpegState *state);

#endif // JPEG_COMMON_H