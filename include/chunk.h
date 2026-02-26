#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>
#include <stddef.h>

typedef struct Chunk {
    int32_t size_x;
    int32_t size_y;
    int32_t size_z;
    uint8_t *blocks;
} Chunk;

int ChunkLoad(const char *path, Chunk *out_chunk);
void ChunkFree(Chunk *chunk);
uint8_t ChunkGet(const Chunk *chunk, int32_t x, int32_t y, int32_t z);
int ChunkIsValidCoord(const Chunk *chunk, int32_t x, int32_t y, int32_t z);

#endif
