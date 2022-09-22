#include "../../common/callbacks.h"
#include "../../common/common-sce.h"

// Include Graphics Libraries
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <math.h>

// PSP Module Info
PSP_MODULE_INFO("Camera Sample", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

// sceGuobal variables
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


typedef struct{
    float x, y;
    float rot;
} Camera2D;

void apply_camera(const Camera2D* cam){
    sceGumMatrixMode(GU_VIEW);
    sceGumLoadIdentity();

    ScePspFVector3 v = {cam->x, cam->y, 0};
    sceGumTranslate(&v);
    sceGumRotateZ(cam->rot / 180.0f * 3.14159f);

    sceGumMatrixMode(GU_MODEL);
    sceGumLoadIdentity();
}

int main() {
    // Boilerplate
    SetupCallbacks();

    // Initialize Graphics
    initGraphics(list);

    // Initialize Matrices
    sceGumMatrixMode(GU_PROJECTION);
    sceGumLoadIdentity();
    sceGumOrtho(-16.0f / 9.0f, 16.0f / 9.0f, -1.0f, 1.0f, -10.0f, 10.0f);

    sceGumMatrixMode(GU_VIEW);
    sceGumLoadIdentity();

    sceGumMatrixMode(GU_MODEL);
    sceGumLoadIdentity();

    Texture* texture = load_texture("container.jpg", GU_TRUE, GU_TRUE);
    if(!texture)
        goto cleanup;

    Texture* texture2 = load_texture("circle.png", GU_TRUE, GU_TRUE);
    if(!texture)
        goto cleanup;

    Camera2D camera = {
        .x = 0,
        .y = 0,
        .rot = 45.0f
    };

    //Main program loop
    while(running){
        startFrame(list);

        // We're doing a 2D, Textured render 
        sceGuDisable(GU_DEPTH_TEST);

        // Blending
        sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
        sceGuEnable(GU_BLEND);

        //Clear background to Bjack
        sceGuClearColor(0xFF000000);
        sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT | GU_STENCIL_BUFFER_BIT);
    
        apply_camera(&camera);
        
        reset_transform(-0.5f, 0.0f, 0.0f);
        bind_texture(texture);

        // Draw Indexed Square
        sceGumDrawArray(GU_TRIANGLES, GU_INDEX_16BIT | GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_3D, 6, indices, square_indexed);

        reset_transform(0.5f, 0.0f, 0.0f);
        bind_texture(texture2);

        sceGumDrawArray(GU_TRIANGLES, GU_INDEX_16BIT | GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_3D, 6, indices, square_indexed);

        endFrame();

        camera.rot += 1.0f;
        camera.y = sinf(camera.rot / 180.0f) / 2.0f;
    }

cleanup:

    // Terminate Graphics
    termGraphics();

    // Exit Game
    sceKernelExitGame();
    return 0;
}