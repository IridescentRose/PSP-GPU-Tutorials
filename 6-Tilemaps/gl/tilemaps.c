#include "../../common/callbacks.h"
#include "../../common/common-gl.h"
#include <gu2gl.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

// PSP Module Info
PSP_MODULE_INFO("Tilemap Sample", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

// Global variables
int running = 1;
static unsigned int __attribute__((aligned(16))) list[262144];

typedef struct {
    void* data;
    u16* indices;
    u32 index_count;
} Mesh;

Mesh* create_mesh(u32 vcount, u32 index_count) {
    Mesh* mesh = malloc(sizeof(Mesh));
    if(mesh == NULL)
        return NULL;
    
    mesh->data = memalign(16, sizeof(Vertex) * vcount);
    if(mesh->data == NULL) {
        free(mesh);
        return NULL;
    }
    mesh->indices = memalign(16, sizeof(u16) * index_count);
    if(mesh->indices == NULL) {
        free(mesh->data);
        free(mesh);
        return NULL;
    }

    mesh->index_count = index_count;

    return mesh;
}

void draw_mesh(Mesh* mesh) {
    glDrawElements(GL_TRIANGLES, GL_INDEX_16BIT | GL_TEXTURE_32BITF | GL_COLOR_8888 | GL_VERTEX_32BITF | GL_TRANSFORM_3D, mesh->index_count, mesh->indices, mesh->data);
}

void destroy_mesh(Mesh* mesh) {
    free(mesh->data);
    free(mesh->indices);
    free(mesh);
}

Vertex create_vert(float u, float v, unsigned int color, float x, float y, float z) {
    Vertex vert = {
        .u = u,
        .v = v,
        .color = color,
        .x = x,
        .y = y,
        .z = z
    };

    return vert;
}

typedef struct {
    float w, h;
} TextureAtlas;

void get_uv_index(TextureAtlas* atlas, float* buf, int idx) {
    int row = idx / (int)atlas->w;
    int column = idx % (int)atlas->h;

    float sizeX = 1.f / ((float)atlas->w);
    float sizeY = 1.f / ((float)atlas->h);

    float y = (float)row * sizeY;
    float x = (float)column * sizeX;
    float h = y + sizeY;
    float w = x + sizeX;

    // 0 0
    // 0 1
    // 1 1
    // 1 0 
    buf[0] = x;
    buf[1] = h;

    buf[2] = x;
    buf[3] = y;

    buf[4] = w;
    buf[5] = y;

    buf[6] = w;
    buf[7] = h;
}

typedef struct {
    int x, y;
    int tex_idx;
} Tile;

typedef struct {
    float x, y;
    float scale_x, scale_y;
    int w, h;
    TextureAtlas atlas;
    Texture* texture;
    Tile* tiles;
    Mesh* mesh;
} Tilemap;

Tilemap* create_tilemap(TextureAtlas atlas, Texture* texture, int sizex, int sizey) {
    Tilemap* tilemap = (Tilemap*)malloc(sizeof(Tilemap));
    if(tilemap == NULL)
        return NULL;

    tilemap->tiles = (Tile*)malloc(sizeof(Tile) * sizex * sizey);
    if(tilemap->tiles == NULL){
        free(tilemap);
        return NULL;
    }
    
    tilemap->mesh = create_mesh(sizex * sizey * 4, sizex * sizey * 6);
    if(tilemap->mesh == NULL){
        free(tilemap->tiles);
        free(tilemap);
    }
    
    memset(tilemap->mesh->data, 0, sizeof(Vertex) * sizex * sizey);
    memset(tilemap->mesh->indices, 0, sizeof(u16) * sizex * sizey);
    memset(tilemap->tiles, 0, sizeof(Tile) * sizex * sizey);

    tilemap->atlas = atlas;
    tilemap->texture = texture;
    tilemap->x = 0;
    tilemap->y = 0;
    tilemap->w = sizex;
    tilemap->h = sizey;
    tilemap->mesh->index_count = tilemap->w * tilemap->h * 6;
    tilemap->scale_x = 16.0f;
    tilemap->scale_y = 16.0f;

    return tilemap;
}

void build_tilemap(Tilemap* tilemap) {
    for(int i = 0; i < tilemap->w * tilemap->h; i++){
        float buf[8];
        get_uv_index(&tilemap->atlas, buf, tilemap->tiles[i].tex_idx);

        float tx = (float)tilemap->tiles[i].x;
        float ty = (float)tilemap->tiles[i].y;
        float tw = tx + 1.0f;
        float th = ty + 1.0f;

        ((Vertex*)tilemap->mesh->data)[i * 4 + 0] = create_vert(buf[0], buf[1], 0xFFFFFFFF, tx, ty, 0.0f);
        ((Vertex*)tilemap->mesh->data)[i * 4 + 1] = create_vert(buf[2], buf[3], 0xFFFFFFFF, tx, th, 0.0f);
        ((Vertex*)tilemap->mesh->data)[i * 4 + 2] = create_vert(buf[4], buf[5], 0xFFFFFFFF, tw, th, 0.0f);
        ((Vertex*)tilemap->mesh->data)[i * 4 + 3] = create_vert(buf[6], buf[7], 0xFFFFFFFF, tw, ty, 0.0f);

        tilemap->mesh->indices[i * 6 + 0] = (i * 4) + 0;
        tilemap->mesh->indices[i * 6 + 1] = (i * 4) + 1;
        tilemap->mesh->indices[i * 6 + 2] = (i * 4) + 2;
        tilemap->mesh->indices[i * 6 + 3] = (i * 4) + 2;
        tilemap->mesh->indices[i * 6 + 4] = (i * 4) + 3;
        tilemap->mesh->indices[i * 6 + 5] = (i * 4) + 0;
    }
    sceKernelDcacheWritebackInvalidateAll();
}

void draw_tilemap(Tilemap* tilemap) {
    glMatrixMode(GL_MODEL);
    glLoadIdentity();

    ScePspFVector3 v = {
        .x = tilemap->x,
        .y = tilemap->y,
        .z = 0.0f,
    };

    gluTranslate(&v);

    ScePspFVector3 v1 = {
        .x = tilemap->scale_x,
        .y = tilemap->scale_y,
        .z = 0.0f,
    };

    gluScale(&v1);

    bind_texture(tilemap->texture);
    draw_mesh(tilemap->mesh);
}

void destroy_tilemap(Tilemap* tilemap) {
    destroy_mesh(tilemap->mesh);
    free(tilemap->tiles);
    free(tilemap);
}

typedef struct{
    float x, y;
    float rot;
} Camera2D;

void apply_camera(const Camera2D* cam){
    glMatrixMode(GL_VIEW);
    glLoadIdentity();

    ScePspFVector3 v = {cam->x, cam->y, 0};
    gluTranslate(&v);
    gluRotateZ(cam->rot / 180.0f * 3.14159f);

    glMatrixMode(GL_MODEL);
    glLoadIdentity();
}

void draw_text(Tilemap* t, const char* str){
    int len = strlen(str);

    for(int i = 0; i < len; i++){
        char c = str[i];

        Tile tile = {
            .x = i % t->w,
            .y = i / t->w,
            .tex_idx = c
        };

        t->tiles[i] = tile;
    }
}

int main() {
    // Boilerplate
    SetupCallbacks();

    // Initialize Graphics
    guglInit(list);

    // Initialize Matrices
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 480, 0.0f, 272.0f, -10.0f, 10.0f);

    glMatrixMode(GL_VIEW);
    glLoadIdentity();

    glMatrixMode(GL_MODEL);
    glLoadIdentity();

    Texture* texture = load_texture("terrain.png", GL_FALSE, GL_TRUE);
    Texture* texture2 = load_texture("default.png", GL_FALSE, GL_TRUE);

    TextureAtlas atlas = {.w = 16, .h = 16};
    Tilemap* tilemap = create_tilemap(atlas, texture, 8, 8);
    Tilemap* tilemap2 = create_tilemap(atlas, texture2, 16, 16);
    tilemap2->x = 144;
    tilemap2->y = 16;

    draw_text(tilemap2, "Hello World!");
    build_tilemap(tilemap2);

    for(int y = 0; y < 8; y++) {
        for(int x = 0; x < 8; x++) {
            Tile tile = {
                .x = x,
                .y = y,
                .tex_idx = x + y * 8
            };
            tilemap->tiles[x + y * 8] = tile;
        }
    }

    tilemap->x = 176;
    tilemap->y = 136;
    build_tilemap(tilemap);

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

        draw_tilemap(tilemap);
        draw_tilemap(tilemap2);

        guglSwapBuffers(GL_TRUE, GL_FALSE);
    }

    destroy_tilemap(tilemap);
    destroy_tilemap(tilemap2);

    // Terminate Graphics
    guglTerm();

    // Exit Game
    sceKernelExitGame();
    return 0;
}