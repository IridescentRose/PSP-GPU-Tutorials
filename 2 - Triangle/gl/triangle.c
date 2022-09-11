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

struct Vertex __attribute__((aligned(16))) triangle[3 * 3] = {
    {0xFF0000FF, -0.5, 0.0, 0.0},
    {0xFF00FF00, 0.5, 0.0, 0.0},
    {0xFFFF0000, 0.0, 0.5, 0},
};

int main() {
    // Boilerplate
    SetupCallbacks();

    // Initialize Graphics
    guglInit(list);
    
    //Main program loop
    while(running){
        guglStartFrame(list, GL_FALSE);
    
        //Clear background to Bjack
        glClearColor(0xFF000000);
        glClear(GL_COLOR_BUFFER_BIT);
    
        glDrawElements(GL_TRIANGLES, GL_COLOR_8888 | GL_VERTEX_32BITF | GL_TRANSFORM_3D, 3, NULL, triangle);

        guglSwapBuffers(GL_TRUE, GL_FALSE);
    }

    // Terminate Graphics
    guglTerm();

    // Exit Game
    sceKernelExitGame();
    return 0;
}