#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include <limits.h>
#include <stdint.h>

#define BYTE_COUNT 256

struct nk_context;

typedef struct {
    uint64_t frequency[BYTE_COUNT];
    char *codes[BYTE_COUNT];
    int count;
    int maxFrequency;
} CompressionData;

typedef struct {
    bool showTabs;
    uint32_t currentTab;
    char droppedPath[PATH_MAX];
} UIState;

typedef struct {
    CompressionData *compressionData;
    UIState *state;
} UI;


typedef void (*CompressCallback) (CompressionData *compressionData, const char *path);

void uiDraw(struct nk_context *ctx, UI *ui, int windowWidth, int windowHeight, CompressCallback compressCallback);






#endif // UI_H
