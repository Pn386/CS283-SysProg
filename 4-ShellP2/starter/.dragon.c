#include <stdio.h>

// Define the structure for run-length encoded data
typedef struct {
    char ch;
    int count;
} dragon_rle_t;

// Run-length encoded dragon data
const dragon_rle_t DRAGON_COMPRESSED[] = {
    {' ', 56}, {'@', 4}, {'\n', 1},
    {' ', 53}, {'%', 6}, {'\n', 1},
    {' ', 52}, {'%', 6}, {'\n', 1},
    {' ', 49}, {'%', 1}, {' ', 1}, {'%', 7}, {' ', 11}, {'@', 1}, {'\n', 1},
    // ... (compressed data continues)
    // Add the rest of the compressed data here
};

const int DRAGON_COMPRESSED_SIZE = sizeof(DRAGON_COMPRESSED) / sizeof(dragon_rle_t);

// Function to print the dragon ASCII art
void print_dragon(void) {
    for (int i = 0; i < DRAGON_COMPRESSED_SIZE; i++) {
        for (int j = 0; j < DRAGON_COMPRESSED[i].count; j++) {
            putchar(DRAGON_COMPRESSED[i].ch);
        }
    }
}

int main() {
    print_dragon();
    return 0;
}
