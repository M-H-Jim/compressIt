#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION

#include <ctype.h>
#include "huffman.h"

#include "nuklear.h"
#include "nuklear_glfw_gl3.h"

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024


int compare(const void *a, const void *b);
void errorCallback(int error_code, const char* description);
void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);




char droppedPath[PATH_MAX] = {0};

static void dropCallback(GLFWwindow* window, int path_count, const char* paths[]) {
    if (path_count > 0) {
        snprintf(droppedPath, sizeof(droppedPath), "%s", paths[0]);
        droppedPath[PATH_MAX - 1] = '\0';
    }
}


int main(void) {
    

    GLint windowWidth  = 0;
    GLint windowHeight = 0;
    GLint framebufferWidth  = 0;
    GLint framebufferHeight = 0;
    
    
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return 1;
    }
    
    glfwSetErrorCallback(errorCallback);
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "compressIt", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return 1;
    }
    
    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetDropCallback(window, dropCallback);
    
    
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    
    
    
    
    // Information
    
    uint64_t frequency[256] = {0};
    char *codes[256] = {NULL};
    int mx = 0;
    int count = 0;
    
    
    //~ qsort(freq, 256, sizeof(int64_t), compare);
    
    
    //
    
    
    struct nk_glfw glfw = {0};
    struct nk_context *ctx = nk_glfw3_init(&glfw, window, NK_GLFW3_INSTALL_CALLBACKS);
    static nk_flags window_flags = NK_WINDOW_TITLE            | 
                               NK_WINDOW_BORDER               |
                               NK_WINDOW_NO_SCROLLBAR;
    // font
    {
        struct nk_font_atlas *atlas;
        nk_glfw3_font_stash_begin(&glfw, &atlas);
        nk_glfw3_font_stash_end(&glfw);
    }
    
    bool showCharts = false;
    
    static int currentTab = 0;
    
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        processInput(window);
        
        glfwGetWindowSize(window, &windowWidth, &windowHeight);
        
        
        
        nk_glfw3_new_frame(&glfw);
        
        
        if (nk_begin(ctx, "CompressIt", nk_rect(0, 0, windowWidth, windowHeight), 
                     NK_WINDOW_BORDER   |
                     NK_WINDOW_TITLE))
        {
            
            
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_label(ctx, "Drop a file here", NK_TEXT_CENTERED);
            if (droppedPath[0] != '\0') {
                nk_layout_row_dynamic(ctx, 30, 2);
                if (nk_button_label(ctx, "Compress It")) {
                    for (int i = 0; i < BYTE_COUNT; i++) {
                        frequency[i] = 0;
                    }
                    freeCodes(codes);
                    mx = 0;
                    count = 0;
                    huffmanCompress(frequency, codes, droppedPath, &count, &mx);
                    showCharts = nk_true;
                    printf("Button pressed\n");
                }
                if (nk_button_label(ctx, "Decompress It")) {
                    printf("button 2 pressed\n");
                }
            }
            if (showCharts) {
                
                static const char *tabNames[] = {"Frequency Graph", "Huffman Codes"};
                
                nk_style_push_vec2(ctx, &ctx->style.window.spacing, nk_vec2(0, 0));
                nk_style_push_float(ctx, &ctx->style.button.rounding, 0);
                
                nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
                
                for (size_t i = 0; i < 2; i++) {
                    const struct nk_user_font *font = ctx->style.font;
                    
                    float textWidth = font->width(font->userdata, font->height, tabNames[i], nk_strlen(tabNames[i]));
                    float widgetWidth = textWidth + 3.0f * ctx->style.button.padding.x;
                    
                    nk_layout_row_push(ctx, widgetWidth);
                    
                    if (currentTab == i) {
                        struct nk_style_item normal = ctx->style.button.normal;
                        ctx->style.button.normal = ctx->style.button.active;
                        if (nk_button_label(ctx, tabNames[i])) currentTab = i;
                        ctx->style.button.normal = normal;
                    }
                    else {
                        if (nk_button_label(ctx, tabNames[i])) currentTab = i;
                    }
                }
                
                nk_layout_row_end(ctx);
                
                nk_style_pop_float(ctx);
                nk_style_pop_vec2(ctx);
                
                
                nk_layout_row_dynamic(ctx, windowHeight - 150, 1);
                
                if (nk_group_begin(ctx, "Notebook", NK_WINDOW_BORDER)) {
                    
                    switch (currentTab) {
                        case 0: {
                            
                            int hovered = -1;
                            
                            nk_layout_row_dynamic(ctx, windowHeight - 180, 1);
                            
                            if (nk_chart_begin(ctx, NK_CHART_COLUMN, count, 0.0f, (float)mx)) {
                                for (int i = 0; i < 256; i++) {
                                    if (frequency[i]) {
                                        nk_flags res = nk_chart_push(ctx, (float)frequency[i]);
                                        if (res & NK_CHART_HOVERING) hovered = i;
                                    }
                                }
                                nk_chart_end(ctx);
                            }
                            
                            if (hovered != -1) {
                                nk_uchar c = (nk_uchar)hovered;
                                if (isprint(c)) {
                                    nk_tooltipf(ctx, "Char: |%c|  Hex: 0x%02X  Frequency: %lld", c, hovered, frequency[hovered]);
                                }
                                else {
                                    nk_tooltipf(ctx, "Hex: 0x%02X  Frequency: %lld", hovered, frequency[hovered]);
                                }
                            }
                            break;
                        }
                        case 1: {
                            nk_layout_row_dynamic(ctx, 25, 3);
                            nk_label(ctx, "Byte", NK_TEXT_LEFT);
                            nk_label(ctx, "Frequency", NK_TEXT_LEFT);
                            nk_label(ctx, "Huffman Code", NK_TEXT_LEFT);
                            
                            
                            for (uint32_t i = 0; i < BYTE_COUNT; i++) {
                                if (frequency[i]) {
                                    
                                    char character[20];
                                    nk_uchar c = i;
                                    
                                    if (isprint(c)) snprintf(character, sizeof(character), "0x%02X  |%c|", i, c);
                                    else snprintf(character, sizeof(character), "0x%02X", i);
                                    
                                    nk_label(ctx, character, NK_TEXT_LEFT);
                                    nk_labelf(ctx, NK_TEXT_LEFT, "%lld", frequency[i]);
                                    nk_label(ctx, codes[i], NK_TEXT_LEFT);
                                }
                            }
                            break;
                        }
                        
                    }
                    nk_group_end(ctx);
                }
            }
        }
        nk_end(ctx);
        
        
        
        float color = 0.1f;
        glClearColor(color, color, color, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        nk_glfw3_render(&glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
        
        glfwSwapBuffers(window);
        
        
        
    }
    
    
    
    freeCodes(codes);
    nk_glfw3_shutdown(&glfw);
    glfwDestroyWindow(window);
    glfwTerminate();
    
    
    
    return 0;
}


int compare(const void *a, const void *b) {
    return (*(uint64_t *)b - *(uint64_t *)a);
}

void errorCallback(int error_code, const char* description) {
    fprintf(stderr, "Error %d: %s\n", error_code, description);
}

void framebufferSizeCallback(GLFWwindow *window, int fbwidth, int fbheight) {
    glViewport(0, 0, fbwidth, fbheight);
}

void processInput(GLFWwindow *window) {
    if (
        glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && 
            (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || 
            glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
    ) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}











