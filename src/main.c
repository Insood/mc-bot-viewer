#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>

#include "chunk.h"

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

static void DrawChunkCubes(const Chunk *chunk)
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
                DrawCube(position, 1.0f, 1.0f, 1.0f, BROWN);
                DrawCubeWires(position, 1.0f, 1.0f, 1.0f, DARKBROWN);
            }
        }
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
        DrawChunkCubes(&chunk);
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
