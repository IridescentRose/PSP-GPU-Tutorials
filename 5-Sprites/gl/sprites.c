#include "../../common/callbacks.h"
#include "../../common/common-gl.h"
#include <gu2gl.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

// PSP Module Info
PSP_MODULE_INFO("Sprite Sample", 0, 1, 1);
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

typedef struct {
    float x, y;
    float rot;
    float sx, sy;

    int layer;

    Mesh* mesh;
    Texture* tex;
} Sprite;

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

Sprite* create_sprite(float x, float y, float sx, float sy, Texture* tex) {
    Sprite* sprite = malloc(sizeof(Sprite));
    if(sprite == NULL)
        return NULL;

    sprite->mesh = create_mesh(4, 6);
    sprite->layer = 0;
    
    if(sprite->mesh == NULL){
        free(sprite);
        return NULL;
    }

    ((Vertex*)sprite->mesh->data)[0] = create_vert(0, 0, 0xFFFFFFFF, -0.25f, -0.25f, 0.0f);
    ((Vertex*)sprite->mesh->data)[1] = create_vert(0, 1, 0xFFFFFFFF, -0.25f,  0.25f, 0.0f);
    ((Vertex*)sprite->mesh->data)[2] = create_vert(1, 1, 0xFFFFFFFF,  0.25f,  0.25f, 0.0f);
    ((Vertex*)sprite->mesh->data)[3] = create_vert(1, 0, 0xFFFFFFFF,  0.25f, -0.25f, 0.0f);
    
    sprite->mesh->indices[0] = 0;
    sprite->mesh->indices[1] = 1;
    sprite->mesh->indices[2] = 2;
    sprite->mesh->indices[3] = 2;
    sprite->mesh->indices[4] = 3;
    sprite->mesh->indices[5] = 0;

    sprite->mesh->index_count = 6;
    sprite->tex = tex;

    sprite->x = x;
    sprite->y = y;
    sprite->sx = sx;
    sprite->sy = sy;

    sceKernelDcacheWritebackInvalidateAll();

    return sprite;
}

void draw_sprite(Sprite* sprite) {
    glMatrixMode(GL_MODEL);
    glLoadIdentity();

    ScePspFVector3 v = {
        .x = sprite->x,
        .y = sprite->y,
        .z = sprite->layer,
    };

    gluTranslate(&v);
    gluRotateZ(sprite->rot);

    ScePspFVector3 s = {
        .x = sprite->sx,
        .y = sprite->sy,
        .z = 1.0f
    };
    gluScale(&s);

    bind_texture(sprite->tex);
    draw_mesh(sprite->mesh);
}

void destroy_sprite(Sprite* sprite) {
    destroy_mesh(sprite->mesh);
    free(sprite);
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

    Texture* texture = load_texture("container.jpg", GL_TRUE, GL_TRUE);
    if(!texture)
        goto cleanup;

    Sprite* sprite = create_sprite(-0.5f, 0.0f, 1.0f, 1.0f, texture);
    if(!sprite)
        goto cleanup;

    Camera2D camera = {
        .x = 0,
        .y = 0,
        .rot = 45.0f
    };

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

        apply_camera(&camera);
    
        draw_sprite(sprite);

        guglSwapBuffers(GL_TRUE, GL_FALSE);

        camera.rot += 1.0f;
        camera.y = sinf(camera.rot / 180.0f) / 2.0f;
    }

cleanup:

    // Terminate Graphics
    guglTerm();

    // Exit Game
    sceKernelExitGame();
    return 0;
}