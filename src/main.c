/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/

#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chunk.h"

static void PrintSliceZ(const Chunk *chunk, int index)
{
    int32_t row = 0;
    int32_t col = 0;

    if (!chunk || !chunk->blocks) {
        printf("No chunk loaded.\n");
        return;
    }

    if (index < 0 || index >= chunk->size_z) {
        printf("Slice Z=%d out of range (0..%d)\n", index, chunk->size_z - 1);
        return;
    }

    for (row = 0; row < chunk->size_x; row += 1) {
        for (col = 0; col < chunk->size_y; col += 1) {
            uint8_t value = ChunkGet(chunk, row, col, index);
            putchar(value == 0 ? ' ' : '#');
        }
        putchar('\n');
    }
}

int main(int argc, char **argv)
{
    Chunk chunk;
    const char *path = NULL;
    if (argc < 2) {
        printf("Usage: %s path/to/file.chunk\n", argv[0]);
        printf("Example: %s chunks/example.chunk\n", argv[0]);
        return 1;
    }

    path = argv[1];

    if (!ChunkLoad(path, &chunk)) {
        printf("Failed to load chunk: %s\n", path);
        return 1;
    }

    printf("Loaded chunk %s (%d x %d x %d)\n", path, chunk.size_x, chunk.size_y, chunk.size_z);
    printf("Printing all Z slices (0..%d)\n", chunk.size_z - 1);

    for (int32_t index = 0; index < chunk.size_z; index += 1) {
        printf("\nZ=%d\n", index);
        PrintSliceZ(&chunk, index);
    }

    ChunkFree(&chunk);
    return 0;
}
