#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include <limits.h>
#include <stdint.h>

#include "huffman.h"

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


//~ typedef struct HuffmanNode HuffmanNode;
typedef struct HuffmanUINode {
    HuffmanNode *node;
    float x;
    float y;
    
    struct HuffmanUINode *left;
    struct HuffmanUINode *right;
} HuffmanUINode;


typedef void (*CompressCallback) (CompressionData *compressionData, const char *path);

void uiDraw(struct nk_context *ctx, UI *ui, int windowWidth, int windowHeight, CompressCallback compressCallback, HuffmanNode **root, HuffmanUINode **uiNode);

static void drawFileSection (struct nk_context *ctx, UI *ui, CompressCallback compressCallback, HuffmanNode **root, HuffmanUINode **uiNode);
static void drawTabs(struct nk_context *ctx, UIState *state);
static void drawTabContent(struct nk_context *ctx, UI *ui, int windowHeight, HuffmanUINode *uiNode);
static void drawFrequencyGraph(struct nk_context *ctx, UI *ui, int windowHeight);
static void drawHuffmanCodes(struct nk_context *ctx, UI *ui);







HuffmanUINode* createUITree(HuffmanNode *node);
void layoutLeaves (HuffmanUINode *node, int *leafIndex);

//~ int leafIndex = 0;
//~ layoutLeaves(uiRoot, &leafIndex);
//~ layoutInternalNodes
//~ layoutY()

void layoutInternalNodes (HuffmanUINode *node);
void layoutY (HuffmanUINode *node, float y);
void freeUITree(HuffmanUINode *node);





typedef struct HuffmanTreeView {
    float zoom;
    
    float panX;
    float panY;
    
    bool dragging;
    
    float dragStartX;
    float dragStartY;
    
    float panStartX;
    float panStartY;
    HuffmanUINode *hoveredNode;
} HuffmanTreeView;

static void treeToScreen(float treeX, float treeY, float *screenX, float *screenY);

















#endif // UI_H
