#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include <limits.h>
#include <stdint.h>

#include "huffman.h"

#define NK_INCLUDE_FONT_BAKING
//~ #define NK_INCLUDE_DEFAULT_FONT

//~ #define BYTE_COUNT 256

struct nk_context;


typedef struct {
    uint64_t    frequency[BYTE_COUNT];
    char        *codes[BYTE_COUNT];
    int         count;
    int         maxFrequency;
} CompressionData;

typedef struct {
    bool        showTabs;
    uint32_t    currentTab;
    char        droppedPath[PATH_MAX];
    bool        decompressionDone;
} UIState;

typedef struct {
    struct nk_font *small;
    struct nk_font *normal;
    struct nk_font *large;
} Fonts;


typedef struct {
    CompressionData*    compressionData;
    UIState*            state;
    Fonts*              fonts;
} UI;

typedef struct HuffmanUINode {
    HuffmanNode*            node;
    float                   x;
    float                   y;
    struct HuffmanUINode*   left;
    struct HuffmanUINode*   right;
} HuffmanUINode;

typedef struct HuffmanTreeView {
    float           zoom;
    float           panX;
    float           panY;
    bool            dragging;
    float           dragStartX;
    float           dragStartY;
    float           panStartX;
    float           panStartY;
    HuffmanUINode*  hoveredNode;
} HuffmanTreeView;


HuffmanUINode*          createHuffmanUiTree (HuffmanNode *node);
static bool             pointInNode         (float x,float y, float nodeX, float nodeY);
static HuffmanUINode*   findHoveredNode     (HuffmanUINode *node, float treeX, float treeY);

void                    layoutLeavesX       (HuffmanUINode *node, int *leafIndex);
void                    layoutInternalNodes (HuffmanUINode *node);
void                    layoutY             (HuffmanUINode *node, float y);
void                    freeHuffmanUiTree   (HuffmanUINode *node);
void                    drawTreeConnections (struct nk_context *ctx, HuffmanUINode *node);
void                    drawTreeNodes       (struct nk_context *ctx, HuffmanUINode *node);

uint64_t                get_fileSize        (const char *path);
void                    zoomAt              (float mouseX, float mouseY, float zoomFactor);

typedef void (*CompressCallback) (CompressionData *compressionData, const char *path);

void                    uiDraw              (struct nk_context *ctx, UI *ui, int windowWidth, int windowHeight, CompressCallback compressCallback, HuffmanNode **root, HuffmanUINode **uiNode);

static void             drawFileSection     (struct nk_context *ctx, UI *ui, CompressCallback compressCallback, HuffmanNode **root, HuffmanUINode **uiNode);
static void             drawTabs            (struct nk_context *ctx, UIState *state);
static void             drawTabContent      (struct nk_context *ctx, UI *ui, int windowHeight, HuffmanUINode *uiNode);
static void             drawStatistics      (struct nk_context *ctx, UI *ui);
static void             drawFrequencyGraph  (struct nk_context *ctx, UI *ui, int windowHeight);
static void             drawHuffmanCodes    (struct nk_context *ctx, UI *ui);

static void             treeToScreen(float treeX, float treeY, float *screenX, float *screenY);
static void             screenToTree(float screenX, float screenY, float *treeX, float *treeY);


#endif // UI_H
