#include "ui.h"

#include <stdio.h>
#include <ctype.h>

#define NK_INCLUDE_STANDARD_VARARGS

#include "nuklear.h"








void uiDraw(struct nk_context *ctx, UI *ui, int windowWidth, int windowHeight, CompressCallback compressCallback) {
    if (!nk_begin(ctx, "CompressIt by M.H.Jim", nk_rect(0, 0, windowWidth, windowHeight), NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
        nk_end(ctx);
        return;
    }
    
    drawFileSection(ctx, ui, compressCallback);
    
    if (ui->state->showTabs) {
        drawTabs(ctx, ui->state);
        nk_layout_row_dynamic(ctx, windowHeight - 150, 1);
        if (nk_group_begin(ctx, "Notebook", NK_WINDOW_BORDER)) {
            drawTabContent(ctx, ui, windowHeight);
            nk_group_end(ctx);
        }
    }
    nk_end(ctx);
}


static void drawFileSection(struct nk_context *ctx, UI *ui, CompressCallback compressCallback) {
    
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
        }
    }
    if (nk_button_label(ctx, "Decompress It")) {
        printf("ctx");
    } 
}

static void drawTabs(struct nk_context *ctx, UIState *state) {
    static const char *tabNames[] = {
        "Frequency Graph",
        "Huffman Codes"
    };
    
    nk_style_push_vec2(ctx, &ctx->style.window.spacing, nk_vec2(0, 0));
    nk_style_push_float(ctx, &ctx->style.button.rounding, 0);
    
    nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
    
    for (uint8_t i = 0; i < 2; i++) {
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

static void drawTabContent(struct nk_context *ctx, UI *ui, int windowHeight) {
    switch (ui->state->currentTab) {
        case 0: {
            drawFrequencyGraph(ctx, ui, windowHeight);
            break;
        }
        case 1: {
            drawHuffmanCodes(ctx, ui);
            break;
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





















