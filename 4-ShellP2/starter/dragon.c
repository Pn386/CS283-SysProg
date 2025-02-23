#include <stdio.h>
#include "dragon.h"

// Run-length encoded dragon data
// Format: {character, count}
const dragon_rle_t DRAGON_COMPRESSED[] = {
    {' ', 56}, {'@', 4}, {'\n', 1},
    {' ', 53}, {'%', 6}, {'\n', 1},
    {' ', 52}, {'%', 6}, {'\n', 1},
    {' ', 49}, {'%', 1}, {' ', 1}, {'%', 7}, {' ', 11}, {'@', 1}, {'\n', 1},
    // ... (compressed data continues)
};

const int DRAGON_COMPRESSED_SIZE = sizeof(DRAGON_COMPRESSED) / sizeof(dragon_rle_t);

void print_dragon(void) {
    for (int i = 0; i < DRAGON_COMPRESSED_SIZE; i++) {
        for (int j = 0; j < DRAGON_COMPRESSED[i].count; j++) {
            putchar(DRAGON_COMPRESSED[i].ch);
        }
    }
}