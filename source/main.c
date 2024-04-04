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
    size_t length;
    byte* buffer;
} BinaryBuffer;

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

bool is_ascii(byte c) {
    return c >= 32 && c <= 127;
}

bool is_low(byte c) {
    return c < 32;
}

bool is_high(byte c) {
    return c > 127;
}

bool is_0xff(byte c) {
    return c == 0xff;
}

bool is_0x00(byte c) {
    return c == 0x00;
}

Color hex_to_color(byte c) {
    if (is_ascii(c)) return BLUE;
    
    if (is_0xff(c)) return WHITE;
    if (is_0x00(c)) return BLACK;

    if (is_low(c)) return GREEN;
    if (is_high(c)) return RED;

    return MAGENTA;
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
}

void display_hex_dump(BinaryBuffer* bin, char* fmt) {
    if (!bin) {
        return;
    }

    for (int i = 0; i < bin->length; i++) {
        printf(fmt, bin->buffer[i]);
    }
}

int main(int argc, char** argv) {
    char* filename = NULL;

    if (argc != 2) {
        printf("!MUST INCLUDE BINFILE TO OPEN: bv [binfile]\n");
        return -1;
    }

    BinaryBuffer bin = create_binary_buffer(argv[1]);
    display_hex_dump(&bin, "%02x ");

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);

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
