#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>

#include "chunk.h"

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

int main(int argc, char **argv)
{
    Chunk chunk;
    const char *path = NULL;
    Color block_colors[256];

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

    InitBlockColors(block_colors);

    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Chunk Viewer");

    Camera3D camera = { 0 };
    Vector3 center = ChunkCenter(&chunk);
    camera.position = (Vector3){ center.x + 20.0f, center.y + 20.0f, center.z + 20.0f };
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

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
        DrawChunkCubes(&chunk, block_colors);
        DrawGrid(10, 1.0f);
        EndMode3D();

        DrawRectangle(10, 10, 340, 80, Fade(SKYBLUE, 0.5f));
        DrawRectangleLines(10, 10, 340, 80, BLUE);
        DrawText("Free camera controls:", 20, 20, 10, BLACK);
        DrawText("- Mouse Wheel to Zoom in-out", 40, 40, 10, DARKGRAY);
        DrawText("- Mouse Wheel Pressed to Pan", 40, 60, 10, DARKGRAY);
        DrawText("- Z to focus center", 40, 80, 10, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();
    ChunkFree(&chunk);
    return 0;
}
