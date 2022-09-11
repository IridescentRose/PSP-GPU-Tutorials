#include "../../common/callbacks.h"

// Include Graphics Libraries
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>

// Define PSP Width / Height
#define PSP_BUF_WIDTH (512)
#define PSP_SCR_WIDTH (480)
#define PSP_SCR_HEIGHT (272)

// PSP Module Info
PSP_MODULE_INFO("Triangle Sample", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

// Global variables
int running = 1;
static unsigned int __attribute__((aligned(16))) list[262144];


// Get Memory Size
static unsigned int getMemorySize(unsigned int width, unsigned int height, unsigned int psm)
{
	switch (psm)
	{
		case GU_PSM_T4:
			return (width * height) >> 1;

		case GU_PSM_T8:
			return width * height;

		case GU_PSM_5650:
		case GU_PSM_5551:
		case GU_PSM_4444:
		case GU_PSM_T16:
			return 2 * width * height;

		case GU_PSM_8888:
		case GU_PSM_T32:
			return 4 * width * height;

		default:
			return 0;
	}
}

// Vram Buffer Request
void* getStaticVramBuffer(unsigned int width, unsigned int height, unsigned int psm)
{
	static unsigned int staticOffset = 0;
	unsigned int memSize = getMemorySize(width,height,psm);
	void* result = (void*)staticOffset;
	staticOffset += memSize;

	return result;
}

// Vram Texture Request
void* getStaticVramTexture(unsigned int width, unsigned int height, unsigned int psm)
{
	void* result = getStaticVramBuffer(width,height,psm);
	return (void*)(((unsigned int)result) + ((unsigned int)sceGeEdramGetAddr()));
}


// Initialize Graphics
void initGraphics() {
    void* fbp0 = getStaticVramBuffer(PSP_BUF_WIDTH,PSP_SCR_HEIGHT,GU_PSM_8888);
	void* fbp1 = getStaticVramBuffer(PSP_BUF_WIDTH,PSP_SCR_HEIGHT,GU_PSM_8888);
	void* zbp = getStaticVramBuffer(PSP_BUF_WIDTH,PSP_SCR_HEIGHT,GU_PSM_4444);

	sceGuInit();

	sceGuStart(GU_DIRECT,list);
	sceGuDrawBuffer(GU_PSM_8888,fbp0,PSP_BUF_WIDTH);
	sceGuDispBuffer(PSP_SCR_WIDTH,PSP_SCR_HEIGHT,fbp1,PSP_BUF_WIDTH);
	sceGuDepthBuffer(zbp,PSP_BUF_WIDTH);
	sceGuOffset(2048 - (PSP_SCR_WIDTH/2),2048 - (PSP_SCR_HEIGHT/2));
	sceGuViewport(2048,2048,PSP_SCR_WIDTH,PSP_SCR_HEIGHT);
	sceGuDepthRange(65535,0);
	sceGuScissor(0,0,PSP_SCR_WIDTH,PSP_SCR_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuDepthFunc(GU_GEQUAL);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuFrontFace(GU_CW);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_CULL_FACE);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuEnable(GU_CLIP_PLANES);
	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);
}

// Start Frame
void startFrame() {
    sceGuStart(GU_DIRECT, list);
}

// End Frame
void endFrame() {
    sceGuFinish();
    sceGuSync(0, 0);
    sceDisplayWaitVblankStart();
	sceGuSwapBuffers();
}

// End Graphics
void termGraphics() {
    sceGuTerm();
}

// Vert Data
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
    sceGumLoadIdentity();
    sceGumTranslate(&v);
}

int main() {
    // Boilerplate
    SetupCallbacks();

    // Initialize Graphics
    initGraphics();

    // Initialize Matrices
    sceGumMatrixMode(GU_PROJECTION);
    sceGumLoadIdentity();
    sceGumOrtho(-16.0f / 9.0f, 16.0f / 9.0f, -1.0f, 1.0f, -10.0f, 10.0f);

    sceGumMatrixMode(GU_VIEW);
    sceGumLoadIdentity();

    sceGumMatrixMode(GU_MODEL);
    sceGumLoadIdentity();

    //Main program loop
    while(running){
        startFrame();

        // We're doing a 2D, Non-Textured render 
        sceGuDisable(GU_DEPTH_TEST);
        sceGuDisable(GU_TEXTURE_2D);

        //Clear background to Bjack
        sceGuClearColor(0xFF000000);
        sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT | GU_STENCIL_BUFFER_BIT);
    
        //Move this left
        reset_transform(-0.5f, 0.0f, 0.0f);

        // Draw Triangle
        sceGumDrawArray(GU_TRIANGLES, GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_3D, 3, NULL, triangle);

        //Move back to origin, and right 0.5f, up 0.25f
        reset_transform(0.5f, 0.25f, 0.0f);

        // Draw Square
        sceGumDrawArray(GU_TRIANGLES, GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_3D, 6, NULL, square);

        //Move back to origin, and down 0.5f
        reset_transform(0.0f, -0.5f, 0.0f);

        // Draw Indexed Square
        sceGumDrawArray(GU_TRIANGLES, GU_INDEX_16BIT | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_3D, 6, indices, square_indexed);

        endFrame();
    }

    // Terminate Graphics
    termGraphics();

    // Exit Game
    sceKernelExitGame();
    return 0;
}