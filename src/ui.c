#include "ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <windows.h>
#include <fileapi.h>

#define NK_INCLUDE_STANDARD_VARARGS

#include "nuklear.h"

#define NODE_WIDTH  150.0f
#define NODE_HEIGHT 50.0f

HuffmanTreeView view = {
    .zoom = 0.5f,
    .panX = 300.0f,
    .panY = 80.0f,
    .dragging = false,
    .dragStartX = 0.0f,
    .dragStartY = 0.0f,
    .panStartX = 0.0f,
    .panStartY = 0.0f,
    .hoveredNode = NULL
};



static void treeToScreen(float treeX, float treeY, float *screenX, float *screenY) {
    *screenX = treeX * view.zoom + view.panX;
    *screenY = treeY * view.zoom + view.panY;
    
}
static void screenToTree(float screenX, float screenY, float *treeX, float *treeY) {
    *treeX = (screenX - view.panX) / view.zoom;
    *treeY = (screenY - view.panY) / view.zoom;
}

static bool pointInNode(float x,float y, float nodeX, float nodeY) {
    return x >= nodeX - NODE_WIDTH / 2.0f &&
           x <= nodeX + NODE_WIDTH / 2.0f &&
           y >= nodeY - NODE_HEIGHT / 2.0f &&
           y <= nodeY + NODE_HEIGHT / 2.0f;
}

static HuffmanUINode *findHoveredNode (HuffmanUINode *node, float treeX, float treeY) {
    if (node == NULL) return NULL;
    if (pointInNode(treeX, treeY, node->x, node->y)) return node;
    HuffmanUINode *found = findHoveredNode(node->left, treeX, treeY);
    if (found) return found;
    return findHoveredNode(node->right, treeX, treeY);
}

HuffmanUINode* createUITree(HuffmanNode *node) {
    if (node == NULL) return NULL;
    
    HuffmanUINode *uiNode = malloc(sizeof(HuffmanUINode));
    
    uiNode->node = node;
    uiNode->x = 0;
    uiNode->y = 0;
    uiNode->left = createUITree(node->left);
    uiNode->right = createUITree(node->right);
    
    return uiNode;
}

void layoutLeaves(HuffmanUINode *node, int *leafIndex) {
    if (node == NULL) return;
    
    if (node->left == NULL && node->right == NULL) {
        node->x = (*leafIndex + 1) * 180.0f;
        (*leafIndex)++;
        return;
    }
    layoutLeaves(node->left, leafIndex);
    layoutLeaves(node->right, leafIndex);
}

void layoutInternalNodes (HuffmanUINode *node) {
    if (node == NULL) return;
    
    layoutInternalNodes(node->left);
    layoutInternalNodes(node->right);
    
    if (node->left == NULL && node->right == NULL) return;
    
    if (node->left && node->right) node->x = (node->left->x + node->right->x) / 2.0f;
    else if (node->left) node->x = node->left->x;
    else if (node->right) node->x = node->right->x;
}

void layoutY(HuffmanUINode *node, float y) {
    if (node == NULL) return;
    node->y = y;
    
    layoutY(node->left, y + 150.0f);
    layoutY(node->right, y + 150.0f);
}

void drawTreeConnections(struct nk_context *ctx, HuffmanUINode *node) {
    struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);
    const struct nk_user_font *font = ctx->style.font;
    if (node == NULL) return;
    float x1, y1;
    treeToScreen(node->x, node->y, &x1, &y1);
    
    if (node->left != NULL) {
        float x2, y2;
        treeToScreen(node->left->x, node->left->y, &x2, &y2);
        
        nk_stroke_line(canvas, x1, y1, x2, y2, 2.0f, nk_rgb(200, 200, 200));
        
        float mx = (x1 + x2) / 2.0f;
        float my = (y1 + y2) / 2.0f;
        
        nk_draw_text(canvas, nk_rect(mx - 5, my - 15, 15, font->height), "0", 1, font, 
                     nk_rgb(255, 255, 255), nk_rgb(255, 255, 255));
        
    }
    if (node->right != NULL) {
        float x2, y2;
        treeToScreen(node->right->x, node->right->y, &x2, &y2);
        nk_stroke_line(canvas, x1, y1, x2, y2, 2.0f, nk_rgb(200, 200, 200));
        
        float mx = (x1 + x2) / 2.0f;
        float my = (y1 + y2) / 2.0f;
        
        nk_draw_text(canvas, nk_rect(mx + 5, my - 15, 15, font->height), "1", 1, font, 
                     nk_rgb(255, 255, 255), nk_rgb(255, 255, 255));
        
    }
    drawTreeConnections(ctx, node->left);
    drawTreeConnections(ctx, node->right);
}



void drawTreeNodes(struct nk_context *ctx, HuffmanUINode *node) {
    struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);
    bool hovered = (node == view.hoveredNode);
    
    if (node == NULL) return;
    float x;
    float y;
    
    treeToScreen(node->x, node->y, &x, &y);
    
    float width = NODE_WIDTH * view.zoom;
    float height = NODE_HEIGHT * view.zoom;
    
    struct nk_rect rect = nk_rect(x - width / 2.0f, y - height / 2.0f, width, height);
    
    bool isLeaf = node->left == NULL && node->right == NULL;
    
    if (isLeaf) {
        if (hovered) {
            nk_fill_rect(canvas, rect, 6.0f, nk_rgb(35, 95, 70));
            nk_stroke_rect(canvas, rect, 6.0f, 3.0f, nk_rgb(70, 210, 150));
            if (isprint(node->node->character)) nk_tooltipf(ctx, "Char: |%c|  Hex: 0x%02X  Frequency: %lld", node->node->character, node->node->character, node->node->frequency);
            else nk_tooltipf(ctx, "Hex: 0x%02X  Frequency: %lld", node->node->character, node->node->frequency);
        }
        else {
            nk_fill_rect(canvas, rect, 6.0f, nk_rgb(30, 75, 55));
            nk_stroke_rect(canvas, rect, 6.0f, 2.0f, nk_rgb(55, 175, 125));
        }
    }
    else {
        if (hovered) {
            nk_fill_rect(canvas, rect, 6.0f, nk_rgb(70, 65, 130));
            nk_stroke_rect(canvas, rect, 6.0f, 3.0f, nk_rgb(155, 150, 255));
            nk_tooltipf(ctx, "Frequency: %lld", node->node->frequency);
        }
        else {
            nk_fill_rect(canvas, rect, 6.0f, nk_rgb(55, 50, 105));
            nk_stroke_rect(canvas, rect, 6.0f, 2.0f, nk_rgb(125, 120, 240));
        }
    }
    
    
    char text[64];
    //~ if (isLeaf) {
        //~ if (isprint(node->node->character)) snprintf(text, sizeof(text), "0x%02X  |%c| : %lld", node->node->character, node->node->character, node->node->frequency);
    //~ }

    if (isLeaf) snprintf(text, sizeof(text), "0x%02X", node->node->character);
    else snprintf(text, sizeof(text), "%lld", node->node->frequency);

    const struct nk_user_font *font = ctx->style.font;
    float textWidth = font->width(font->userdata, font->height, text, strlen(text));
    nk_draw_text(canvas, nk_rect(x - textWidth / 2.0f, y - font->height / 2.0f, textWidth, font->height),
                 text, strlen(text), font, nk_rgb(255, 255, 255), nk_rgb(255, 255, 255));
    
    drawTreeNodes(ctx, node->left);
    drawTreeNodes(ctx, node->right);
    
}


void freeUITree (HuffmanUINode *node) {
    if (node == NULL) return;
    freeUITree(node->left);
    freeUITree(node->right);
    free(node);
}





















void uiDraw(struct nk_context *ctx, UI *ui, int windowWidth, int windowHeight, CompressCallback compressCallback, HuffmanNode **root, HuffmanUINode **uiNode) {
    if (!nk_begin(ctx, "CompressIt by M.H.Jim", nk_rect(0, 0, windowWidth, windowHeight), NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
        nk_end(ctx);
        return;
    }
    
    drawFileSection(ctx, ui, compressCallback, root, uiNode);
    
    if (ui->state->showTabs) {
        drawTabs(ctx, ui->state);
        nk_layout_row_dynamic(ctx, windowHeight - 150, 1);
        if (nk_group_begin(ctx, "Notebook", NK_WINDOW_BORDER)) {
            drawTabContent(ctx, ui, windowHeight, *uiNode);
            nk_group_end(ctx);
        }
    }
    nk_end(ctx);
}

LARGE_INTEGER start;
LARGE_INTEGER end;
LARGE_INTEGER frequency;

double compressionTime;

static void drawFileSection(struct nk_context *ctx, UI *ui, CompressCallback compressCallback, HuffmanNode **root, HuffmanUINode **uiNode) {
    
    if (ui->state->droppedPath[0] == '\0') {
        nk_layout_row_dynamic(ctx, 50, 1);
        nk_label(ctx, "Drop a file here!!!", NK_TEXT_ALIGN_CENTERED);
        return;
    }
    
    nk_layout_row_dynamic(ctx, 30, 1);
    
    nk_label(ctx, ui->state->droppedPath, NK_TEXT_ALIGN_CENTERED);
    
    nk_layout_row_dynamic(ctx, 30, 2);
    
    if (nk_button_label(ctx, "Compress It")) {
        if (compressCallback) {
            
            QueryPerformanceFrequency(&frequency);
            QueryPerformanceCounter(&start);
            
            compressCallback(ui->compressionData, ui->state->droppedPath);
            
            QueryPerformanceCounter(&end);
            compressionTime = (double)(end.QuadPart - start.QuadPart) / (double)frequency.QuadPart;
            
            ui->state->showTabs = nk_true;
            
            printf("inside->%p\n", *root);
            
            
            
            freeUITree(*uiNode);//////
            
            *uiNode = createUITree(*root);
            
            int leafIndex = 0;
            layoutLeaves(*uiNode, &leafIndex);
            layoutInternalNodes(*uiNode);
            layoutY(*uiNode, 150.0);
            
            
            
            for (int i = 0; i < 10; i++) {
                printf("%f", (*uiNode)->x);
            }
            
            
            
            
            
            
        }
    }
    if (nk_button_label(ctx, "Decompress It")) {
        printf("ctx");
        huffmanDecompress(ui->state->droppedPath);
    } 
}

static void drawTabs(struct nk_context *ctx, UIState *state) {
    static const char *tabNames[] = {
        "Statistics",
        "Frequency Graph",
        "Huffman Codes",
        "Visualization"
    };
    
    nk_style_push_vec2(ctx, &ctx->style.window.spacing, nk_vec2(0, 0));
    nk_style_push_float(ctx, &ctx->style.button.rounding, 0);
    
    nk_layout_row_begin(ctx, NK_STATIC, 30, 4);
    
    for (uint8_t i = 0; i < 4; i++) {
        const struct nk_user_font *font = ctx->style.font;
        
        float textWidth = font->width(font->userdata, font->height, tabNames[i], nk_strlen(tabNames[i]));
        float widgetWidth = textWidth + 3.0f * ctx->style.button.padding.x;
        
        nk_layout_row_push(ctx, widgetWidth);
        
        
        if (state->currentTab == i) {
            struct nk_style_item normal = ctx->style.button.normal;
            ctx->style.button.normal = ctx->style.button.active;
            if (nk_button_label(ctx, tabNames[i])) state->currentTab = i;
            ctx->style.button.normal = normal;
        }
        else {
            if (nk_button_label(ctx, tabNames[i])) state->currentTab = i;
        }
    }
    
    nk_layout_row_end(ctx);
    
    nk_style_pop_float(ctx);
    nk_style_pop_vec2(ctx);
}


void zoomAt (float mouseX, float mouseY, float zoomFactor) {
    float oldZoom  = view.zoom;
    float treeX = (mouseX - view.panX) / oldZoom;
    float treeY = (mouseY - view.panY) / oldZoom;
    float newZoom = oldZoom * zoomFactor;
    if (newZoom < 0.1f) newZoom = 0.1f;
    if (newZoom > 0.7f) newZoom = 0.7f;
    view.zoom = newZoom;
    view.panX = mouseX - treeX * newZoom;
    view.panY = mouseY - treeY * newZoom;
}


uint64_t get_fileSize (const char *path) {
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &data)) return 0;
    return ((uint64_t)data.nFileSizeHigh << 32) | (uint64_t)data.nFileSizeLow;
}




static void drawTabContent(struct nk_context *ctx, UI *ui, int windowHeight, HuffmanUINode *uiNode) {
    switch (ui->state->currentTab) {
        case 0: {
            
            nk_style_push_color(ctx, &ctx->style.text.color, nk_rgb(255, 255, 255));
            
            
            
            const char *originalPath = ui->state->droppedPath;
            char compressedPath[PATH_MAX];
            snprintf(compressedPath, sizeof(compressedPath), "%s.huff", originalPath);
            
            uint64_t originalSize = get_fileSize(originalPath);
            uint64_t compressedSize = get_fileSize(compressedPath);
            
            
            char originalText[64];
            char compressedText[64];

            snprintf(originalText, sizeof(originalText), "%llu bytes", (unsigned long long)originalSize);
            snprintf(compressedText, sizeof(compressedText), "%llu bytes", (unsigned long long)compressedSize);

            nk_layout_row_dynamic(ctx, 35, 1);
            nk_label(ctx, "Compression Statistics", NK_TEXT_CENTERED);
            
            nk_layout_row_dynamic(ctx, 90, 2);

            if (nk_group_begin(ctx, "Original", NK_WINDOW_BORDER)) {
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, "ORIGINAL", NK_TEXT_CENTERED);

                nk_layout_row_dynamic(ctx, 35, 1);
                nk_label(ctx, originalText, NK_TEXT_CENTERED);
            }
            nk_group_end(ctx);

            if (nk_group_begin(ctx, "Compressed", NK_WINDOW_BORDER)) {
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, "COMPRESSED", NK_TEXT_CENTERED);

                nk_layout_row_dynamic(ctx, 35, 1);
                nk_label(ctx, compressedText, NK_TEXT_CENTERED);
            }
            nk_group_end(ctx);
            
            
            uint64_t savedBytes = originalSize > compressedSize ? originalSize - compressedSize : 0;
            
            double savedPercent =
                ((double)savedBytes / (double)originalSize) * 100.0;
            
            double ratio =
                (double)originalSize / (double)compressedSize;
            
            char savedText[64];
            char percentText[64];
            char ratioText[64];
            
            snprintf(savedText, sizeof(savedText), "%lld Bytes", savedBytes);
            
            snprintf(percentText, sizeof(percentText), "%.1f%% smaller", savedPercent);
            
            snprintf(ratioText, sizeof(ratioText), "%.2f : 1", ratio);
            if (nk_group_begin(ctx, "Compress Stats", NK_WINDOW_BORDER)) {
                nk_layout_row_dynamic(ctx, 30, 3);
                
                nk_label(ctx, "SPACE SAVED", NK_TEXT_CENTERED);
                nk_label(ctx, "COMPRESSION", NK_TEXT_CENTERED);
                nk_label(ctx, "RATIO", NK_TEXT_CENTERED);
                
                nk_layout_row_dynamic(ctx, 35, 3);
                
                nk_label(ctx, savedText, NK_TEXT_CENTERED);
                nk_label(ctx, percentText, NK_TEXT_CENTERED);
                nk_label(ctx, ratioText, NK_TEXT_CENTERED);
            }
            nk_group_end(ctx);
            
            char timeText[64];
            
            snprintf(timeText, sizeof(timeText), "%.3f seconds", compressionTime);
            
            if (nk_group_begin(ctx, "Time stats", NK_WINDOW_BORDER)) {
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, "Time", NK_TEXT_CENTERED);
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, timeText, NK_TEXT_CENTERED);
            }
            nk_group_end(ctx);
            
            
            nk_style_pop_color(ctx);
            
            
            
            break;
        }
        case 1: {
            drawFrequencyGraph(ctx, ui, windowHeight);
            break;
        }
        case 2: {
            drawHuffmanCodes(ctx, ui);
            break;
        }
        case 3: {
            
            const struct nk_input in = ctx->input;
            if(in.mouse.buttons[NK_BUTTON_MIDDLE].down) {
                float mouseX = in.mouse.pos.x;
                float mouseY = in.mouse.pos.y;
                
                if (!view.dragging) {
                    view.dragging = true;
                    view.dragStartX = mouseX;
                    view.dragStartY = mouseY;
                    view.panStartX = view.panX;
                    view.panStartY = view.panY;
                }
                else {
                    view.panX = view.panStartX + (mouseX - view.dragStartX);
                    view.panY = view.panStartY + (mouseY - view.dragStartY);
                }
            }
            else {
                view.dragging = false;
            }
            
            if (in.mouse.scroll_delta.y > 0) {
                float mouseX = in.mouse.pos.x;
                float mouseY = in.mouse.pos.y;
                zoomAt(mouseX, mouseY, 1.1f);
            }
            if (in.mouse.scroll_delta.y < 0) {
                float mouseX = in.mouse.pos.x;
                float mouseY = in.mouse.pos.y;
                zoomAt(mouseX, mouseY, 0.9f);
            }
            
            float treeX;
            float treeY;
            
            screenToTree(in.mouse.pos.x, in.mouse.pos.y, &treeX, &treeY);
            view.hoveredNode = findHoveredNode(uiNode, treeX, treeY);
            
            drawTreeConnections(ctx, uiNode);
            drawTreeNodes(ctx, uiNode);
        }
    }
}



static void drawFrequencyGraph(struct nk_context *ctx, UI *ui, int windowHeight) {
    int hovered = -1;
    
    nk_layout_row_dynamic(ctx, windowHeight - 200, 1);
    if (nk_chart_begin(ctx, NK_CHART_COLUMN, ui->compressionData->count, 0.0f, (float)ui->compressionData->maxFrequency)) {
        for (uint16_t i = 0; i < BYTE_COUNT; i++) {
            if (ui->compressionData->frequency[i]) {
                nk_flags result = nk_chart_push(ctx, (float)ui->compressionData->frequency[i]);
                if (result & NK_CHART_HOVERING) hovered = i;
            }
        }
        nk_chart_end(ctx);
    }
    nk_layout_row_dynamic(ctx, 25, 1);
    nk_label(ctx, "X-axis: Byte Values, Y-axis: Frequency", NK_TEXT_CENTERED);
    if (hovered != -1) {
        nk_uchar c = (nk_uchar)hovered;
        if (isprint(c)) {
            //~ nk_tooltipf(ctx, "Char: |%c|  Hex: 0x%02X  \nFrequency: %lld", c, hovered, ui->compressionData->frequency[hovered]);
            if (nk_tooltip_begin(ctx, 150)) {
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_labelf(ctx, NK_TEXT_LEFT, "Character: %c", c);
                nk_labelf(ctx, NK_TEXT_LEFT, "Hex Value: 0x%02X", hovered);
                nk_labelf(ctx, NK_TEXT_LEFT, "Frequency: %lld", ui->compressionData->frequency[hovered]);
            }
            nk_tooltip_end(ctx);
        }
        else {
            if (nk_tooltip_begin(ctx, 150)) {
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_labelf(ctx, NK_TEXT_LEFT, "Hex Value: 0x%02X", hovered);
                nk_labelf(ctx, NK_TEXT_LEFT, "Frequency: %lld", ui->compressionData->frequency[hovered]);
            }
            nk_tooltip_end(ctx);
        }
    }
    
}

static void drawHuffmanCodes(struct nk_context *ctx, UI *ui) {
    
    struct nk_color old_color = ctx->style.text.color;
    ctx->style.text.color = nk_rgb(255, 255, 255);
    
    nk_layout_row_dynamic(ctx, 25, 3);
    nk_label(ctx, "Byte", NK_TEXT_LEFT);
    nk_label(ctx, "Frequency", NK_TEXT_LEFT);
    nk_label(ctx, "Huffman Code", NK_TEXT_LEFT);
    
    
    for (uint32_t i = 0; i < BYTE_COUNT; i++) {
        if (!ui->compressionData->frequency[i]) continue;
            
        char character[20];
        nk_uchar c = i;
        
        if (isprint(c)) snprintf(character, sizeof(character), "0x%02X  |%c|", i, c);
        else snprintf(character, sizeof(character), "0x%02X", i);
        
        nk_label(ctx, character, NK_TEXT_LEFT);
        nk_labelf(ctx, NK_TEXT_LEFT, "%lld", ui->compressionData->frequency[i]);
        
        //~ char code[256];
        //~ snprintf(code, sizeof(code), "%s", ui->compressionData->codes[i]);
        //~ nk_flags flags = NK_EDIT_FIELD | NK_EDIT_READ_ONLY;
        //~ nk_edit_string(ctx, flags, code, &(int){(int)strlen(code)}, sizeof(code), nk_filter_default);
        
        
        nk_label(ctx, ui->compressionData->codes[i], NK_TEXT_LEFT);
        
    }
    ctx->style.text.color = old_color;
}





















