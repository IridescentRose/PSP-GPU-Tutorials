#define STB_IMAGE_IMPLEMENTATION
#include "../../common/stb_image.h"

#include <pspgu.h>
#include <pspgum.h>
#include <pspge.h>
#include <pspdisplay.h>

#include <pspkernel.h>
#include "common-sce.h"

void swizzle_fast(u8 *out, const u8 *in, const unsigned int width, const unsigned int height) {
    unsigned int blockx, blocky;
    unsigned int j;

    unsigned int width_blocks = (width / 16);
    unsigned int height_blocks = (height / 8);

    unsigned int src_pitch = (width - 16) / 4;
    unsigned int src_row = width * 8;

    const u8 *ysrc = in;
    u32 *dst = (u32 *)out;

    for (blocky = 0; blocky < height_blocks; ++blocky) {
        const u8 *xsrc = ysrc;
        for (blockx = 0; blockx < width_blocks; ++blockx) {
            const u32 *src = (u32 *)xsrc;
            for (j = 0; j < 8; ++j) {
                *(dst++) = *(src++);
                *(dst++) = *(src++);
                *(dst++) = *(src++);
                *(dst++) = *(src++);
                src += src_pitch;
            }
            xsrc += 16;
        }
        ysrc += src_row;
    }
}

unsigned int pow2(const unsigned int value) {
    unsigned int poweroftwo = 1;
    while (poweroftwo < value) {
        poweroftwo <<= 1;
    }
    return poweroftwo;
}

void copy_texture_data(void* dest, const void* src, const int pW, const int width, const int height){
    for (unsigned int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {
            ((unsigned int*)dest)[x + y * pW] = ((unsigned int *)src)[x + y * width];
        }
    }
}
Texture* load_texture(const char* filename, const int flip, const int vram) {
    int width, height, nrChannels;    
    stbi_set_flip_vertically_on_load(GU_TRUE);
    unsigned char *data = stbi_load(filename, &width, &height,
                                    &nrChannels, STBI_rgb_alpha);

    if(!data)
        return NULL;

    Texture* tex = (Texture*)malloc(sizeof(Texture));
    tex->width = width;
    tex->height = height;
    tex->pW = pow2(width);
    tex->pH = pow2(height);

    unsigned int *dataBuffer =
        (unsigned int *)memalign(16, tex->pH * tex->pW * 4);

    // Copy to Data Buffer
    copy_texture_data(dataBuffer, data, tex->pW, tex->width, tex->height);

    // Free STB Data
    stbi_image_free(data);

    unsigned int* swizzled_pixels = NULL;
    size_t size = tex->pH * tex->pW * 4;
    if(vram){
        swizzled_pixels = getStaticVramTexture(tex->pW, tex->pH, GU_PSM_8888);
    } else {
        swizzled_pixels = (unsigned int *)memalign(16, size);
    }
    
    swizzle_fast((u8*)swizzled_pixels, (const u8*)dataBuffer, tex->pW * 4, tex->pH);

    free(dataBuffer);
    tex->data = swizzled_pixels;

    sceKernelDcacheWritebackInvalidateAll();

    return tex;
}

void bind_texture(Texture* tex) {
    if(tex == NULL)
        return;

    sceGuTexMode(GU_PSM_8888, 0, 0, 1);
    sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
    sceGuTexFilter(GU_NEAREST, GU_NEAREST);
    sceGuTexWrap(GU_REPEAT, GU_REPEAT);
    sceGuTexImage(0, tex->pW, tex->pH, tex->pW, tex->data);
}

void reset_transform(float x, float y, float z){
    ScePspFVector3 v = {x, y, z};
    sceGumLoadIdentity();
    sceGumTranslate(&v);
}


// Get Memory Size
unsigned int getMemorySize(unsigned int width, unsigned int height, unsigned int psm)
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
void initGraphics(void* list) {
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
void startFrame(void* list) {
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
