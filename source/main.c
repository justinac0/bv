#include <stdio.h>
#include <stdlib.h>

#include <raylib.h>

#define BOX_SIZE 4
#define ROW_WIDTH 256
#define WINDOW_WIDTH (BOX_SIZE * ROW_WIDTH) 
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE "bv"

typedef unsigned char byte;

typedef struct BinaryBuffer {
    int length;
    byte* buffer;
} BinaryBuffer;

// Currently a whole file is loaded into RAM,
// it would be nice to progressively load larger
// files. So that there is no size limit to what binarys
// can be viewed.
BinaryBuffer create_binary_buffer(char* filename) {
    BinaryBuffer bin;
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to read file: %s\n", filename);
        return (BinaryBuffer){0, NULL};
    }

    fseek(file, sizeof(byte), SEEK_END);
    bin.length = ftell(file);

    rewind(file);

    bin.buffer = (byte*) malloc(sizeof(byte) * bin.length);
    if (!bin.buffer) {
        printf("Failed to allocate memory for buffer...\n");
        fclose(file);
        return (BinaryBuffer){0, NULL};
    }

    fread(bin.buffer, sizeof(byte), bin.length, file);
    fclose(file);

    return bin;
}

void destroy_binary_buffer(BinaryBuffer* bin) {
    if (!bin) {
        return;
    }

    bin->length = 0;
    free(bin->buffer);
}

void display_binary_buffer(BinaryBuffer* bin, int x, int y, const int row_width, const int box_size) {
    if (!bin) {
        return;
    }

    int scroll_offset = abs(y / box_size);
    int y_max = WINDOW_HEIGHT / box_size + scroll_offset;

    int render_start = scroll_offset * row_width - row_width;
    int render_end = y_max * row_width + row_width;
    if (render_end > bin->length) {
        render_end = bin->length;
    }

    for (int i = render_start; i < render_end; i++) {
        const int row = i / row_width;
        const int col = i % row_width;

        Color color = (Color){bin->buffer[i], bin->buffer[i], bin->buffer[i], 255};

        DrawRectangle(
            col * box_size + x, row * box_size + y,
            box_size, box_size,
            color
        );
    }

    char str[64];

    DrawRectangle(0, WINDOW_HEIGHT - 30, WINDOW_WIDTH, 30, BLUE);
    sprintf(str, "addr: %04x -> %04x", scroll_offset * row_width, y_max * row_width);
    DrawText(str, 2, WINDOW_HEIGHT - 30 + 4, 24, WHITE);
}

int main(int argc, char** argv) {
    char* filename = NULL;

    if (argc != 2) {
        printf("!MUST INCLUDE BINFILE TO OPEN: bv [binfile]\n");
        return -1;
    }

    BinaryBuffer bin = create_binary_buffer(argv[1]);

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    int scroll_y = 0;

    Camera2D camera;
    camera.offset = (Vector2){
        0, 0
    };
    camera.target = (Vector2){0, 0};
    camera.rotation = 0.0;
    camera.zoom = 1.0;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(MAGENTA);
        
        BeginMode2D(camera);
        display_binary_buffer(&bin, 0, scroll_y, ROW_WIDTH, BOX_SIZE);
        EndMode2D();

        EndDrawing();

        float scroll_delta = GetMouseWheelMove() * 150;
        scroll_y += scroll_delta;
        if (scroll_y > 0) scroll_y = 0;

        if (IsKeyPressed(KEY_R)) {
            scroll_y = 0;
        }
    }

    destroy_binary_buffer(&bin);    
    CloseWindow();

    return 0;
}
