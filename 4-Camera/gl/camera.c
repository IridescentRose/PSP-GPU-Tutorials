#include "../../common/callbacks.h"
#include "../../common/common-gl.h"

#include <gu2gl.h>

#include <string.h>
#include <malloc.h>


// PSP Module Info
PSP_MODULE_INFO("Camera Sample", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

// Global variables
int running = 1;
static unsigned int __attribute__((aligned(16))) list[262144];

Vertex __attribute__((aligned(16))) square_indexed[4] = {
    {0.0f, 0.0f, 0xFF00FFFF, -0.25f, -0.25f, -1.0f},
    {1.0f, 0.0f, 0xFFFF00FF, -0.25f, 0.25f, -1.0f},
    {1.0f, 1.0f, 0xFFFFFF00, 0.25f, 0.25f, -1.0f},
    {0.0f, 1.0f, 0xFF000000, 0.25f, -0.25f, -1.0f},
};

unsigned short __attribute__((aligned(16))) indices[6] = {
    0, 1, 2, 2, 3, 0
};


int main() {
    // Boilerplate
    SetupCallbacks();

    // Initialize Graphics
    guglInit(list);

    // Initialize Matrices
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-16.0f / 9.0f, 16.0f / 9.0f, -1.0f, 1.0f, -10.0f, 10.0f);

    glMatrixMode(GL_VIEW);
    glLoadIdentity();

    glMatrixMode(GL_MODEL);
    glLoadIdentity();

    Texture* texture = load_texture("container.jpg", GL_TRUE, GL_TRUE);
    if(!texture)
        goto cleanup;

    Texture* texture2 = load_texture("circle.png", GL_TRUE, GL_TRUE);
    if(!texture)
        goto cleanup;

    //Main program loop
    while(running){
        guglStartFrame(list, GL_FALSE);

        // We're doing a 2D, Textured render 
        glDisable(GL_DEPTH_TEST);

        // Blending
        glBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
        glEnable(GL_BLEND);

        //Clear background to Bjack
        glClearColor(0xFF000000);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
        reset_transform(-0.5f, 0.0f, 0.0f);
        bind_texture(texture);

        // Draw Indexed Square
        glDrawElements(GL_TRIANGLES, GL_INDEX_16BIT | GL_TEXTURE_32BITF | GL_COLOR_8888 | GL_VERTEX_32BITF | GL_TRANSFORM_3D, 6, indices, square_indexed);

        reset_transform(0.5f, 0.0f, 0.0f);
        bind_texture(texture2);

        glDrawElements(GL_TRIANGLES, GL_INDEX_16BIT | GL_TEXTURE_32BITF | GL_COLOR_8888 | GL_VERTEX_32BITF | GL_TRANSFORM_3D, 6, indices, square_indexed);

        guglSwapBuffers(GL_TRUE, GL_FALSE);
    }

cleanup:

    // Terminate Graphics
    guglTerm();

    // Exit Game
    sceKernelExitGame();
    return 0;
}