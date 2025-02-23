#ifndef __DRAGON_H__
#define __DRAGON_H__

// Compressed dragon data using run-length encoding
typedef struct {
    char ch;
    int count;
} dragon_rle_t;

// Compressed dragon data
extern const dragon_rle_t DRAGON_COMPRESSED[];
extern const int DRAGON_COMPRESSED_SIZE;

// Function to decompress and print the dragon
void print_dragon(void);

#endif