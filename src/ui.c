#include "ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define NK_INCLUDE_STANDARD_VARARGS

#include "nuklear.h"


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
        node->x = (*leafIndex + 1) * 20.0f;
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
    
    layoutY(node->left, y + 50.0f);
    layoutY(node->right, y + 50.0f);
}

void drawTreeConnections(struct nk_command_buffer *canvas, HuffmanUINode *node) {
    if (node == NULL) return;
    if (node->left != NULL) {
        nk_stroke_line(canvas, node->x, node->y, node->left->x, node->left->y, 2.0f, nk_rgb(200, 200, 200));
    }
    if (node->right != NULL) {
        nk_stroke_line(canvas, node->x, node->y, node->right->x, node->right->y, 2.0f, nk_rgb(200, 200, 200));
    }
    drawTreeConnections(canvas, node->left);
    drawTreeConnections(canvas, node->right);
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
            compressCallback(ui->compressionData, ui->state->droppedPath);
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
    } 
}

static void drawTabs(struct nk_context *ctx, UIState *state) {
    static const char *tabNames[] = {
        "Frequency Graph",
        "Huffman Codes",
        "Visualization"
    };
    
    nk_style_push_vec2(ctx, &ctx->style.window.spacing, nk_vec2(0, 0));
    nk_style_push_float(ctx, &ctx->style.button.rounding, 0);
    
    nk_layout_row_begin(ctx, NK_STATIC, 30, 3);
    
    for (uint8_t i = 0; i < 3; i++) {
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

static void drawTabContent(struct nk_context *ctx, UI *ui, int windowHeight, HuffmanUINode *uiNode) {
    switch (ui->state->currentTab) {
        case 0: {
            drawFrequencyGraph(ctx, ui, windowHeight);
            break;
        }
        case 1: {
            drawHuffmanCodes(ctx, ui);
            break;
        }
        case 2: {
            drawTreeConnections(nk_window_get_canvas(ctx), uiNode);
        }
    }
}



static void drawFrequencyGraph(struct nk_context *ctx, UI *ui, int windowHeight) {
    int hovered = -1;
    
    nk_layout_row_dynamic(ctx, windowHeight - 180, 1);
    if (nk_chart_begin(ctx, NK_CHART_COLUMN, ui->compressionData->count, 0.0f, (float)ui->compressionData->maxFrequency)) {
        for (uint16_t i = 0; i < BYTE_COUNT; i++) {
            if (ui->compressionData->frequency[i]) {
                nk_flags result = nk_chart_push(ctx, (float)ui->compressionData->frequency[i]);
                if (result & NK_CHART_HOVERING) hovered = i;
            }
        }
        nk_chart_end(ctx);
    }
    
    if (hovered != -1) {
        nk_uchar c = (nk_uchar)hovered;
        if (isprint(c)) {
            nk_tooltipf(ctx, "Char: |%c|  Hex: 0x%02X  Frequency: %lld", c, hovered, ui->compressionData->frequency[hovered]);
        }
        else {
            nk_tooltipf(ctx, "Hex: 0x%02X  Frequency: %lld", hovered, ui->compressionData->frequency[hovered]);
        }
    }
    
}

static void drawHuffmanCodes(struct nk_context *ctx, UI *ui) {
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
        nk_label(ctx, ui->compressionData->codes[i], NK_TEXT_LEFT);
        
    }

    
    
    
    
    
}





















