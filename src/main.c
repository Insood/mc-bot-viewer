#include "raylib.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "chunk.h"

typedef struct ChunkEntry {
    char *path;
    Chunk chunk;
} ChunkEntry;

static Color ColorFrom01(float r, float g, float b)
{
    Color color;
    color.r = (unsigned char)(r * 255.0f + 0.5f);
    color.g = (unsigned char)(g * 255.0f + 0.5f);
    color.b = (unsigned char)(b * 255.0f + 0.5f);
    color.a = 255;
    return color;
}

static void InitBlockColors(Color *colors)
{
    for (int i = 0; i < 256; i += 1) {
        colors[i] = GRAY;
    }

    colors[0] = (Color){ 0, 0, 0, 0 };
    colors[1] = GRAY;                       // stone
    colors[3] = (Color){ 121, 85, 58, 255 }; // dirt
    colors[4] = DARKGRAY;                   // cobblestone
    colors[5] = (Color){ 160, 130, 90, 255 }; // wood
    colors[6] = GREEN;                      // sappling
    colors[12] = (Color){ 194, 178, 128, 255 }; // sand
    colors[13] = (Color){ 120, 120, 120, 255 }; // gravel
    colors[14] = GOLD;                      // gold
    colors[15] = LIGHTGRAY;                 // iron
    colors[16] = (Color){ 30, 30, 30, 255 }; // coal
    colors[17] = (Color){ 110, 80, 50, 255 }; // treetrunk
    colors[18] = (Color){ 70, 140, 70, 255 }; // leaves
    colors[19] = (Color){ 240, 230, 120, 255 }; // sponge
    colors[20] = (Color){ 120, 180, 255, 180 }; // glass

    colors[21] = ColorFrom01(1.0f, 0.0f, 0.0f); // red
    colors[22] = ORANGE;                    // orange
    colors[23] = YELLOW;                    // yellow
    colors[24] = (Color){ 150, 255, 150, 255 }; // lightgreen
    colors[25] = ColorFrom01(0.0f, 1.0f, 0.0f); // green
    colors[26] = (Color){ 0, 200, 200, 255 }; // aqua
    colors[27] = SKYBLUE;                   // cyan
    colors[28] = BLUE;                      // blue
    colors[29] = PURPLE;                    // purple
    colors[30] = (Color){ 75, 0, 130, 255 }; // indigo
    colors[31] = (Color){ 238, 130, 238, 255 }; // violet
    colors[32] = MAGENTA;                   // magenta
    colors[33] = PINK;                      // pink
    colors[34] = BLACK;                     // black
    colors[35] = GRAY;                      // grey
    colors[36] = RAYWHITE;                  // white

    colors[37] = YELLOW;                    // yellow flower
    colors[38] = ColorFrom01(1.0f, 0.0f, 0.0f); // rose
    colors[39] = ColorFrom01(1.0f, 0.0f, 0.0f); // red mushroom
    colors[40] = (Color){ 120, 80, 50, 255 }; // brown mushroom
    colors[41] = GOLD;                      // gold block
    colors[42] = LIGHTGRAY;                 // ironblock
    colors[44] = (Color){ 110, 90, 70, 255 }; // stair
    colors[45] = (Color){ 150, 60, 50, 255 }; // brick
    colors[46] = (Color){ 200, 30, 30, 255 }; // tnt
    colors[47] = (Color){ 170, 130, 80, 255 }; // bookcase
    colors[48] = (Color){ 90, 120, 90, 255 }; // mossy cobblestone
    colors[49] = (Color){ 30, 10, 50, 255 }; // obsidian
}

static int HasChunkExtension(const char *name)
{
    size_t len = 0;
    if (!name) {
        return 0;
    }
    len = strlen(name);
    return len >= 6 && strcmp(name + (len - 6), ".chunk") == 0;
}

static int IsRegularFile(const char *path)
{
    struct stat info;
    if (!path) {
        return 0;
    }
    if (stat(path, &info) != 0) {
        return 0;
    }
    return S_ISREG(info.st_mode);
}

static char *JoinPath(const char *dir, const char *name)
{
    size_t dir_len = 0;
    size_t name_len = 0;
    size_t needs_sep = 0;
    char *result = NULL;

    if (!dir || !name) {
        return NULL;
    }

    dir_len = strlen(dir);
    name_len = strlen(name);
    needs_sep = (dir_len > 0 && dir[dir_len - 1] != '/') ? 1 : 0;

    result = (char *)malloc(dir_len + needs_sep + name_len + 1);
    if (!result) {
        return NULL;
    }

    memcpy(result, dir, dir_len);
    if (needs_sep) {
        result[dir_len] = '/';
    }
    memcpy(result + dir_len + needs_sep, name, name_len);
    result[dir_len + needs_sep + name_len] = '\0';
    return result;
}

static void FreeChunkEntries(ChunkEntry *entries, size_t count)
{
    if (!entries) {
        return;
    }

    for (size_t i = 0; i < count; i += 1) {
        ChunkFree(&entries[i].chunk);
        free(entries[i].path);
    }
    free(entries);
}

static int LoadChunksFromDirectory(const char *dir, ChunkEntry **out_entries, size_t *out_count)
{
    DIR *handle = NULL;
    struct dirent *entry = NULL;
    ChunkEntry *entries = NULL;
    size_t count = 0;
    size_t capacity = 0;

    if (!dir || !out_entries || !out_count) {
        return 0;
    }

    handle = opendir(dir);
    if (!handle) {
        return 0;
    }

    while ((entry = readdir(handle)) != NULL) {
        char *full_path = NULL;

        if (entry->d_name[0] == '.') {
            continue;
        }

        if (!HasChunkExtension(entry->d_name)) {
            continue;
        }

        full_path = JoinPath(dir, entry->d_name);
        if (!full_path) {
            continue;
        }

        if (!IsRegularFile(full_path)) {
            free(full_path);
            continue;
        }

        if (count == capacity) {
            size_t next_capacity = capacity == 0 ? 8 : capacity * 2;
            ChunkEntry *next_entries = (ChunkEntry *)realloc(entries, next_capacity * sizeof(ChunkEntry));
            if (!next_entries) {
                free(full_path);
                FreeChunkEntries(entries, count);
                closedir(handle);
                return 0;
            }
            entries = next_entries;
            capacity = next_capacity;
        }

        if (!ChunkLoad(full_path, &entries[count].chunk)) {
            free(full_path);
            continue;
        }

        entries[count].path = full_path;
        count += 1;
    }

    closedir(handle);

    if (count == 0) {
        FreeChunkEntries(entries, count);
        return 0;
    }

    *out_entries = entries;
    *out_count = count;
    return 1;
}

static Vector3 ChunkCenter(const Chunk *chunk)
{
    Vector3 center = { 0.0f, 0.0f, 0.0f };
    if (!chunk || !chunk->blocks) {
        return center;
    }

    center.x = ((float)chunk->size_x - 1.0f) * 0.5f;
    center.y = ((float)chunk->size_y - 1.0f) * 0.5f;
    center.z = ((float)chunk->size_z - 1.0f) * 0.5f;
    return center;
}

static void DrawChunkCubes(const Chunk *chunk, const Color *colors)
{
    if (!chunk || !chunk->blocks) {
        return;
    }

    for (int32_t x = 0; x < chunk->size_x; x += 1) {
        for (int32_t y = 0; y < chunk->size_y; y += 1) {
            for (int32_t z = 0; z < chunk->size_z; z += 1) {
                uint8_t value = ChunkGet(chunk, x, y, z);
                if (value == 0) {
                    continue;
                }

                Vector3 position = { (float)x, (float)y, (float)z };
                Color color = colors ? colors[value] : BROWN;
                DrawCube(position, 1.0f, 1.0f, 1.0f, color);
                DrawCubeWires(position, 1.0f, 1.0f, 1.0f, Fade(BLACK, 0.35f));
            }
        }
    }
}

static const char *BaseName(const char *path)
{
    const char *slash = NULL;
    if (!path) {
        return "";
    }
    slash = strrchr(path, '/');
    return slash ? slash + 1 : path;
}

int main(int argc, char **argv)
{
    ChunkEntry *entries = NULL;
    size_t entry_count = 0;
    size_t current_index = 0;
    const char *dir = NULL;
    Color block_colors[256];

    if (argc < 2) {
        dir = "chunks";
        printf("No chunk directory provided. Defaulting to '%s'.\n", dir);
    } else {
        dir = argv[1];
    }
    if (!LoadChunksFromDirectory(dir, &entries, &entry_count)) {
        printf("Failed to load any chunks from: %s\n", dir);
        return 1;
    }

    InitBlockColors(block_colors);

    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Chunk Viewer");

    Camera3D camera = { 0 };
    Vector3 center = ChunkCenter(&entries[current_index].chunk);
    camera.position = (Vector3){
        center.x + (float)entries[current_index].chunk.size_x,
        center.y + (float)entries[current_index].chunk.size_y,
        center.z + (float)entries[current_index].chunk.size_z
    };
    camera.target = center;
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    DisableCursor();
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        UpdateCamera(&camera, CAMERA_FREE);

        if (IsKeyPressed(KEY_Z)) {
            camera.target = center;
        }

        if (IsKeyPressed(KEY_SPACE)) {
            current_index = (current_index + 1) % entry_count;
            center = ChunkCenter(&entries[current_index].chunk);
            camera.position = (Vector3){
                center.x + (float)entries[current_index].chunk.size_x,
                center.y + (float)entries[current_index].chunk.size_y,
                center.z + (float)entries[current_index].chunk.size_z
            };
            camera.target = center;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
        DrawChunkCubes(&entries[current_index].chunk, block_colors);
        EndMode3D();

        DrawRectangle(10, 10, 360, 115, Fade(SKYBLUE, 0.5f));
        DrawRectangleLines(10, 10, 360, 115, BLUE);
        DrawText("Free camera controls:", 20, 20, 10, BLACK);
        DrawText("- Mouse Wheel to Zoom in-out", 40, 40, 10, DARKGRAY);
        DrawText("- Mouse Wheel Pressed to Pan", 40, 60, 10, DARKGRAY);
        DrawText("- Z to focus center", 40, 80, 10, DARKGRAY);
        DrawText("- SPACE to next chunk", 40, 100, 10, DARKGRAY);
        DrawText(BaseName(entries[current_index].path), 20, 115, 10, BLACK);

        EndDrawing();
    }

    CloseWindow();
    FreeChunkEntries(entries, entry_count);
    return 0;
}
