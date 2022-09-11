#include "../../common/callbacks.h"
#define GUGL_IMPLEMENTATION
#include <gu2gl.h>

// PSP Module Info
PSP_MODULE_INFO("Triangle Sample", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

// Global variables
int running = 1;
static unsigned int __attribute__((aligned(16))) list[262144];

struct Vertex
{
	unsigned int color;
	float x, y, z;
};

struct Vertex __attribute__((aligned(16))) triangle[3] = {
    {0xFF00FF00, 0.35f, 0.0, -1.0f},
    {0xFF0000FF, -0.35f, 0.0, -1.0f},
    {0xFFFF0000, 0.0, 0.5f, -1.0f},
};

struct Vertex __attribute__((aligned(16))) square[6] = {
    {0xFF00FF00, -0.25f, -0.25f, -1.0f},
    {0xFF0000FF, -0.25f, 0.25f, -1.0f},
    {0xFFFF0000, 0.25f, 0.25f, -1.0f},
    
    {0xFFFF0000, 0.25f, 0.25f, -1.0f},
    {0xFFFFFFFF, 0.25f, -0.25f, -1.0f},
    {0xFF00FF00, -0.25f, -0.25f, -1.0f},
};

struct Vertex __attribute__((aligned(16))) square_indexed[4] = {
    {0xFF00FFFF, -0.25f, -0.25f, -1.0f},
    {0xFFFF00FF, -0.25f, 0.25f, -1.0f},
    {0xFFFFFF00, 0.25f, 0.25f, -1.0f},
    {0xFF000000, 0.25f, -0.25f, -1.0f},
};

unsigned short __attribute__((aligned(16))) indices[6] = {
    0, 1, 2, 2, 3, 0
};

/**
 * @brief Resets matrix and applies transform
 * 
 */
void reset_transform(float x, float y, float z){
    ScePspFVector3 v = {x, y, z};
    glLoadIdentity();
    gluTranslate(&v);
}

int main() {
    // Boilerplate
    SetupCallbacks();

    // Initialize Graphics
    guglInit(list);

    // Initialize Matrices
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-16.0f / 9.0f, 16.0f / 9.0f, -1.0f, 1.0f, 10.0f, -10.0f);

    glMatrixMode(GL_VIEW);
    glLoadIdentity();

    glMatrixMode(GL_MODEL);
    glLoadIdentity();

    //Main program loop
    while(running){
        guglStartFrame(list, GL_FALSE);

        // We're doing a 2D, Non-Textured render 
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_TEXTURE_2D);

        //Clear background to Bjack
        glClearColor(0xFF000000);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
        //Move this left
        reset_transform(-0.5f, 0.0f, 0.0f);

        // Draw Triangle
        glDrawElements(GL_TRIANGLES, GL_COLOR_8888 | GL_VERTEX_32BITF | GL_TRANSFORM_3D, 3, NULL, triangle);

        //Move back to origin, and right 0.5f, up 0.25f
        reset_transform(0.5f, 0.25f, 0.0f);

        // Draw Square
        glDrawElements(GL_TRIANGLES, GL_COLOR_8888 | GL_VERTEX_32BITF | GL_TRANSFORM_3D, 6, NULL, square);

        //Move back to origin, and down 0.5f
        reset_transform(0.0f, -0.5f, 0.0f);

        // Draw Indexed Square
        glDrawElements(GL_TRIANGLES, GL_INDEX_16BIT | GL_COLOR_8888 | GL_VERTEX_32BITF | GL_TRANSFORM_3D, 6, indices, square_indexed);

        guglSwapBuffers(GL_TRUE, GL_FALSE);
    }

    // Terminate Graphics
    guglTerm();

    // Exit Game
    sceKernelExitGame();
    return 0;
}