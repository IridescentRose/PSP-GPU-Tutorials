#ifndef COMMON_GL_H_INCLUDED
#define COMMON_GL_H_INCLUDED

#if __cplusplus__
extern "C"{
#endif

typedef struct
{
    float u, v;
	unsigned int color;
	float x, y, z;
} Vertex;

typedef struct {
    unsigned int width, height;
    unsigned int pW, pH;
    void* data;
}Texture;

void reset_transform(float x, float y, float z);
Texture* load_texture(const char* filename, const int flip, const int vram);
void bind_texture(Texture* tex);

#if __cplusplus__
};
#endif

#endif