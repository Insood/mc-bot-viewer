#include "chunk.h"

#include <stdio.h>
#include <stdlib.h>

static void ChunkReset(Chunk *chunk)
{
    if (!chunk) {
        return;
    }
    chunk->size_x = 0;
    chunk->size_y = 0;
    chunk->size_z = 0;
    chunk->blocks = NULL;
}

static int ReadI32BE(FILE *file, int32_t *out_value)
{
    unsigned char buffer[4];
    if (fread(buffer, 1, 4, file) != 4) {
        return 0;
    }
    *out_value = (int32_t)((uint32_t)buffer[0] << 24 |
                           (uint32_t)buffer[1] << 16 |
                           (uint32_t)buffer[2] << 8  |
                           (uint32_t)buffer[3]);
    return 1;
}

int ChunkLoad(const char *path, Chunk *out_chunk)
{
    FILE *file = NULL;
    int32_t size_x = 0;
    int32_t size_y = 0;
    int32_t size_z = 0;
    size_t total = 0;
    size_t bytes_read = 0;
    uint8_t *blocks = NULL;

    if (!path || !out_chunk) {
        return 0;
    }

    ChunkReset(out_chunk);

    file = fopen(path, "rb");
    if (!file) {
        return 0;
    }

    if (!ReadI32BE(file, &size_x) || !ReadI32BE(file, &size_y) || !ReadI32BE(file, &size_z)) {
        fclose(file);
        return 0;
    }

    if (size_x <= 0 || size_y <= 0 || size_z <= 0) {
        fclose(file);
        return 0;
    }

    if ((size_t)size_x > SIZE_MAX / (size_t)size_y) {
        fclose(file);
        return 0;
    }
    if ((size_t)size_x * (size_t)size_y > SIZE_MAX / (size_t)size_z) {
        fclose(file);
        return 0;
    }

    total = (size_t)size_x * (size_t)size_y * (size_t)size_z;
    if (total == 0) {
        fclose(file);
        return 0;
    }

    blocks = (uint8_t *)malloc(total);
    if (!blocks) {
        fclose(file);
        return 0;
    }

    bytes_read = fread(blocks, 1, total, file);
    fclose(file);

    if (bytes_read != total) {
        free(blocks);
        return 0;
    }

    out_chunk->size_x = size_x;
    out_chunk->size_y = size_y;
    out_chunk->size_z = size_z;
    out_chunk->blocks = blocks;
    return 1;
}

void ChunkFree(Chunk *chunk)
{
    if (!chunk) {
        return;
    }

    free(chunk->blocks);
    ChunkReset(chunk);
}

int ChunkIsValidCoord(const Chunk *chunk, int32_t x, int32_t y, int32_t z)
{
    if (!chunk || !chunk->blocks) {
        return 0;
    }

    return x >= 0 && x < chunk->size_x &&
           y >= 0 && y < chunk->size_y &&
           z >= 0 && z < chunk->size_z;
}

uint8_t ChunkGet(const Chunk *chunk, int32_t x, int32_t y, int32_t z)
{
    size_t index = 0;
    if (!ChunkIsValidCoord(chunk, x, y, z)) {
        return 0;
    }

    index = ((size_t)x * (size_t)chunk->size_y + (size_t)y) * (size_t)chunk->size_z + (size_t)z;
    return chunk->blocks[index];
}
