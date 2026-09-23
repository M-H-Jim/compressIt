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

#include "huffman.h"

#include "ui.h"

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



CompressionData compressionData = {0};
UIState state = {0};
Fonts fonts = {0};
UI ui = {
    .compressionData = &compressionData,
    .state = &state,
    .fonts = &fonts
};

HuffmanNode *root = NULL;
HuffmanUINode *uiNode = NULL;




static void dropCallback(GLFWwindow* window, int path_count, const char* paths[]) {
    if (path_count > 0) {
        snprintf(state.droppedPath, sizeof(state.droppedPath), "%s", paths[0]);
        state.droppedPath[PATH_MAX - 1] = '\0';
    }
}



void compressCallback(CompressionData *compressionData, const char *path) {
    memset(compressionData->frequency, 0, sizeof(compressionData->frequency));
    freeCodes(compressionData->codes);
    compressionData->maxFrequency = 0;
    compressionData->count = 0;
    
    
    freeHuffmanTree(root);
    
    huffmanCompress(compressionData->frequency, compressionData->codes, path, &compressionData->count, &compressionData->maxFrequency, &root);
    //~ printf("a->%p\n", root);

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
    
    
    
    
    
    struct nk_glfw glfw = {0};
    struct nk_context *ctx = nk_glfw3_init(&glfw, window, NK_GLFW3_INSTALL_CALLBACKS);
    
    // font
    {
        //~ struct nk_font_atlas *atlas;
        //~ nk_glfw3_font_stash_begin(&glfw, &atlas);
        //~ nk_glfw3_font_stash_end(&glfw);
        
        struct nk_font_atlas *atlas;

        nk_glfw3_font_stash_begin(&glfw, &atlas);

        ui.fonts->small  = nk_font_atlas_add_default(atlas, 16.0f, NULL);
        ui.fonts->normal = nk_font_atlas_add_default(atlas, 20.0f, NULL);
        ui.fonts->large  = nk_font_atlas_add_default(atlas, 26.0f, NULL);

        nk_glfw3_font_stash_end(&glfw);

        nk_style_set_font(ctx, &ui.fonts->small->handle);

        
        
        
        
        
    }
    
    nk_style_push_color(ctx, &ctx->style.text.color, nk_rgb(255, 255, 255));
    
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        processInput(window);
        
        glfwGetWindowSize(window, &windowWidth, &windowHeight);
        
        
        
        nk_glfw3_new_frame(&glfw);
        
        
        uiDraw(ctx, &ui, windowWidth, windowHeight, compressCallback, &root, &uiNode);
        
        
        
        float color = 0.1f;
        glClearColor(color, color, color, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        nk_glfw3_render(&glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
        
        glfwSwapBuffers(window);
        
        
        
    }
    
    
    
    freeCodes(compressionData.codes);
    freeHuffmanUiTree(uiNode);
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











