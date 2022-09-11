#include "../../common/callbacks.h"
#define GUGL_IMPLEMENTATION
#include <gu2gl.h>

// PSP Module Info
PSP_MODULE_INFO("Context Sample", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

// Global variables
int running = 1;
static unsigned int __attribute__((aligned(16))) list[262144];

int main() {
    // Boilerplate
    SetupCallbacks();

    // Initialize Graphics
    guglInit(list);
    
    //Main program loop
    while(running){
        guglStartFrame(list, GL_FALSE);
    
        //Clear background to Green
        glClearColor(0xFF00FF00);
        glClear(GL_COLOR_BUFFER_BIT);
    
        guglSwapBuffers(GL_TRUE, GL_FALSE);
    }

    // Terminate Graphics
    guglTerm();

    // Exit Game
    sceKernelExitGame();
    return 0;
}