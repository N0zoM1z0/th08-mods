#include <emscripten/emscripten.h>
#include <emscripten/heap.h>
#include <stdint.h>
#include <stdio.h>

namespace
{
int InspectFile(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "unable to open %s\n", path);
        return 1;
    }

    if (fseek(file, 0, SEEK_END) != 0)
    {
        fprintf(stderr, "unable to seek %s\n", path);
        fclose(file);
        return 1;
    }

    long size = ftell(file);
    unsigned char first = 0;
    unsigned char last = 0;
    if (size <= 0 || fseek(file, 0, SEEK_SET) != 0 || fread(&first, 1, 1, file) != 1 ||
        fseek(file, size - 1, SEEK_SET) != 0 || fread(&last, 1, 1, file) != 1)
    {
        fprintf(stderr, "unable to sample %s\n", path);
        fclose(file);
        return 1;
    }

    fclose(file);
    printf("%s: size=%ld first=%02x last=%02x\n", path, size, first, last);
    return 0;
}
}

extern "C" EMSCRIPTEN_KEEPALIVE int inspect_data_files()
{
    int result = InspectFile("/game/th08.dat");
    result |= InspectFile("/game/thbgm.dat");
    printf("wasm-heap-bytes=%lu\n", static_cast<unsigned long>(emscripten_get_heap_size()));
    return result;
}
