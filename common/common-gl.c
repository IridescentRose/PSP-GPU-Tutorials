#define STB_IMAGE_IMPLEMENTATION
#include "../../common/stb_image.h"

#define GUGL_IMPLEMENTATION
#include <gu2gl.h>
#include <pspkernel.h>
#include <malloc.h>
#include <string.h>
#include "common-gl.h"


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
    stbi_set_flip_vertically_on_load(flip);
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