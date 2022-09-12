#include "../../common/callbacks.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../common/stb_image.h"

#include <string.h>
#include <malloc.h>

#define GUGL_IMPLEMENTATION
#include <gu2gl.h>


// PSP Module Info
PSP_MODULE_INFO("Texture Sample", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

// Global variables
int running = 1;
static unsigned int __attribute__((aligned(16))) list[262144];

struct Vertex
{
    float u, v;
	unsigned int color;
	float x, y, z;
};

struct Vertex __attribute__((aligned(16))) square_indexed[4] = {
    {0.0f, 0.0f, 0xFF00FFFF, -0.25f, -0.25f, -1.0f},
    {1.0f, 0.0f, 0xFFFF00FF, -0.25f, 0.25f, -1.0f},
    {1.0f, 1.0f, 0xFFFFFF00, 0.25f, 0.25f, -1.0f},
    {0.0f, 1.0f, 0xFF000000, 0.25f, -0.25f, -1.0f},
};

unsigned short __attribute__((aligned(16))) indices[6] = {
    0, 1, 2, 2, 3, 0
};

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

typedef struct {
    unsigned int width, height;
    unsigned int pW, pH;
    void* data;
}Texture;

Texture* load_texture(const char* filename, const int vram) {
    int width, height, nrChannels;    
    stbi_set_flip_vertically_on_load(GL_TRUE);
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

    glTexMode(GU_PSM_8888, 0, 0, 1);
    glTexFunc(GL_TFX_MODULATE, GL_TCC_RGBA);
    glTexFilter(GL_NEAREST, GL_NEAREST);
    glTexWrap(GL_REPEAT, GL_REPEAT);
    glTexImage(0, tex->pW, tex->pH, tex->pW, tex->data);
}

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
    glOrtho(-16.0f / 9.0f, 16.0f / 9.0f, -1.0f, 1.0f, -10.0f, 10.0f);

    glMatrixMode(GL_VIEW);
    glLoadIdentity();

    glMatrixMode(GL_MODEL);
    glLoadIdentity();

    Texture* texture = load_texture("container.jpg", GL_TRUE);
    if(!texture)
        goto cleanup;

    Texture* texture2 = load_texture("circle.png", GL_TRUE);
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