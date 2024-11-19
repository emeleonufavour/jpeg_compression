#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jpeg_common.h>

#define MAX_SYMBOLS 256
#define MAX_TREE_NODES (MAX_SYMBOLS * 2 - 1)

// The Huffman tree node structure
typedef struct HuffmanNode
{
    int symbol;    // Original symbol (or -1 for internal nodes)
    int frequency; // Frequency of the symbol
    int left;      // Index of left child
    int right;     // Index of right child
    char *code;    // Huffman code for the symbol
} HuffmanNode;

// The structure to store symbol frequencies
typedef struct
{
    int symbol;
    int frequency;
} SymbolFreq;

// Compare function for qsort
int compare_frequencies(const void *a, const void *b)
{
    return ((SymbolFreq *)a)->frequency - ((SymbolFreq *)b)->frequency;
}

// Build the Huffman tree from RLE codes
HuffmanNode *build_huffman_tree(RLECode *rle_data, int code_count)
{
    // Count frequencies of (run_length, value) pairs
    SymbolFreq frequencies[MAX_SYMBOLS] = {0};
    int unique_symbols = 0;

    // Count frequencies of RLE codes
    for (int i = 0; i < code_count; i++)
    {
        // Create a unique symbol from run_length and value
        // We'll use upper 8 bits for run_length and lower 8 bits for value
        int symbol = (rle_data[i].run_length << 8) | (abs(rle_data[i].value) & 0xFF);

        // Find or add symbol
        int found = 0;
        for (int j = 0; j < unique_symbols; j++)
        {
            if (frequencies[j].symbol == symbol)
            {
                frequencies[j].frequency++;
                found = 1;
                break;
            }
        }
        if (!found)
        {
            frequencies[unique_symbols].symbol = symbol;
            frequencies[unique_symbols].frequency = 1;
            unique_symbols++;
        }
    }

    // Sort by frequency
    qsort(frequencies, unique_symbols, sizeof(SymbolFreq), compare_frequencies);

    // Create tree nodes
    HuffmanNode *tree = (HuffmanNode *)calloc(MAX_TREE_NODES, sizeof(HuffmanNode));
    int node_count = 0;

    // Initialize leaf nodes
    for (int i = 0; i < unique_symbols; i++)
    {
        tree[node_count].symbol = frequencies[i].symbol;
        tree[node_count].frequency = frequencies[i].frequency;
        tree[node_count].left = -1;
        tree[node_count].right = -1;
        tree[node_count].code = NULL;
        node_count++;
    }

    // Build tree by combining nodes
    while (node_count < unique_symbols * 2 - 1)
    {
        // Find two nodes with lowest frequencies
        int min1 = 0, min2 = 1;
        for (int i = 2; i < node_count; i++)
        {
            if (tree[i].frequency < tree[min1].frequency)
            {
                min2 = min1;
                min1 = i;
            }
            else if (tree[i].frequency < tree[min2].frequency)
            {
                min2 = i;
            }
        }

        // Create new internal node
        tree[node_count].symbol = -1;
        tree[node_count].frequency = tree[min1].frequency + tree[min2].frequency;
        tree[node_count].left = min1;
        tree[node_count].right = min2;
        tree[node_count].code = NULL;
        node_count++;
    }

    return tree;
}

// Generate Huffman codes recursively
void generate_huffman_codes(HuffmanNode *tree, int node_index, char *current_code)
{
    if (node_index == -1)
        return;

    // If this is a leaf node, store the code
    if (tree[node_index].left == -1 && tree[node_index].right == -1)
    {
        tree[node_index].code = strdup(current_code);
        return;
    }

    // Traverse left (add '0')
    char left_code[256];
    sprintf(left_code, "%s0", current_code);
    generate_huffman_codes(tree, tree[node_index].left, left_code);

    // Traverse right (add '1')
    char right_code[256];
    sprintf(right_code, "%s1", current_code);
    generate_huffman_codes(tree, tree[node_index].right, right_code);
}

// Function to encode RLE data using Huffman coding
void huffman_encode_rle(RLECode *rle_data, int code_count)
{
    // Build Huffman tree
    HuffmanNode *tree = build_huffman_tree(rle_data, code_count);

    // Generate Huffman codes
    generate_huffman_codes(tree, code_count * 2 - 2, "");

    // Print Huffman codes for each symbol
    printf("\nHuffman Codes:\n");
    for (int i = 0; i < code_count * 2 - 1; i++)
    {
        if (tree[i].symbol != -1 && tree[i].code != NULL)
        {
            int run_length = (tree[i].symbol >> 8) & 0xFF;
            int value = tree[i].symbol & 0xFF;
            printf("(%d zeros, %d): %s\n", run_length, value, tree[i].code);
        }
    }

    // Clean up
    for (int i = 0; i < code_count * 2 - 1; i++)
    {
        free(tree[i].code);
    }
    free(tree);
}