#include <cglm/struct/vec2.h>
#include <cglm/struct/vec4.h>

#include <stdio.h>
#include <math.h>

#include "gl_core_3_3.h"

#include "clist.h"
#include "util.h"
#include "sprite.h"

// todo make this lot static!
static GLuint SpriteProgram = 0;
static GLuint SpriteVAO     = 0;
static GLuint SpriteVBO     = 0;

static GLuint SpriteProjL  = 0;
static GLuint SpriteSizeL  = 0;
static GLuint SpriteAtlasL = 0;
static GLuint SpritePosL   = 0;
static GLuint SpriteRotL   = 0;
static GLuint SpriteTexL   = 0;
static GLuint SpriteTintL  = 0;

static GLuint SpriteTexture = 0;

static float  SpriteData[5 * sizeof(float)];

// not static so other headers can extern it if needed
clist_t*      SpriteList;

Sprite* newSprite(vec4s area, int cpType)
{
    Sprite* s = calloc(1, sizeof(Sprite));

    s->pos.x = rnd(area.x, area.y);
    s->pos.y = rnd(area.z, area.w);

    s->size = (vec2s) { { 32, 32 } };
    s->rot  = 0;
    s->tex  = 96;

    switch (cpType) {
    case 0:
        s->tint = (vec4s) { { 1, 0, 0, 1 } };
        break;
    case 1:
        s->tint = (vec4s) { { 1, 0, 1, 1 } };
        break;
    case 2:
    default:
        s->tint = (vec4s) { { 0, 1, 1, 1 } };
        break;
    case 3:
        s->tint = (vec4s) { { 0, 1, 0, 1 } };
        break;
    }

    s->otint = s->tint;

    s->dragOff  = (vec2s)GLMS_VEC2_ZERO_INIT;
    s->dragging = false;
    clistAddNode(SpriteList, s);
    return(s);
}

void initSprites()
{
    SpriteList    = clistCreateList();
    SpriteProgram = createProgramGlsl("data/sprite.glsl", true);
    glUseProgram(SpriteProgram);

    SpriteProjL  = glGetUniformLocation(SpriteProgram, "projection");
    SpriteAtlasL = glGetUniformLocation(SpriteProgram, "texture0");
    SpriteTexL   = glGetUniformLocation(SpriteProgram, "tex");
    SpriteTintL  = glGetUniformLocation(SpriteProgram, "in_tint");

    SpriteSizeL = glGetAttribLocation(SpriteProgram, "in_size");
    SpritePosL  = glGetAttribLocation(SpriteProgram, "in_position");
    SpriteRotL  = glGetAttribLocation(SpriteProgram, "in_rotation");

    glGenVertexArrays(1, &SpriteVAO);
    glBindVertexArray(SpriteVAO);
    glGenBuffers(1, &SpriteVBO);

    glBindBuffer(GL_ARRAY_BUFFER, SpriteVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SpriteData), SpriteData, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(SpritePosL);
    glEnableVertexAttribArray(SpriteSizeL);
    glEnableVertexAttribArray(SpriteRotL);

    glVertexAttribPointer(SpritePosL, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteData), 0);
    glVertexAttribPointer(SpriteSizeL, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteData), (void*)8);
    glVertexAttribPointer(SpriteRotL, 1, GL_FLOAT, GL_FALSE, sizeof(SpriteData), (void*)16);

    // can currently only have one atlas, have a way to load multiple
    // and select which one to use.
    //SpriteTexture = loadTextureAtlas("data/atlas.png", 256, 256, 6);
    SpriteTexture = loadTextureAtlas("data/font.png", 64, 64, 97);
    glUniform1i(SpriteAtlasL, 0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, SpriteTexture);
    glCheckError(__FILE__, __LINE__);
}

// have a beginSprites that sets proj and activates the buffers etc
// and an endSprites that switches them all off...
// so beginsprites, render all sprites, endsprites....

void setSpritePerspective(float* p)
{
    glUseProgram(SpriteProgram);
    glUniformMatrix4fv(SpriteProjL, 1, GL_FALSE, p);
    glActiveTexture(GL_TEXTURE0);
}

static void derefAndDraw(cnode_t* node)
{
    Sprite* s = (Sprite*)(node->data);

    renderSprite(s);
}

void SpriteRenderAll(float* proj)
{
    setSpritePerspective(proj);
    clistIterateForward(SpriteList, derefAndDraw);
}

void renderSprite(Sprite* s)
{
    glUseProgram(SpriteProgram);
    glUniform1i(SpriteTexL, s->tex);
    glUniform4f(SpriteTintL, s->tint.r, s->tint.g, s->tint.b, s->tint.a);

    float* data = &SpriteData[0];
    data[0] = s->pos.x;
    data[1] = s->pos.y;
    data[2] = s->size.x;
    data[3] = s->size.y;
    data[4] = s->rot;

    glBindVertexArray(SpriteVAO);
    glBindBuffer(GL_ARRAY_BUFFER, SpriteVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SpriteData), &SpriteData[0], GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(SpritePosL);
    glEnableVertexAttribArray(SpriteSizeL);
    glEnableVertexAttribArray(SpriteRotL);

    glDrawArrays(GL_POINTS, 0, 1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(SpritePosL);
    glDisableVertexAttribArray(SpriteSizeL);
    glDisableVertexAttribArray(SpriteRotL);
}

bool SpriteInBounds(Sprite* s, float x, float y)
{
    // "unrotate" the sprite... (reverse rotate the mouse)
    float tx = x - s->pos.x;
    float ty = y - s->pos.y;
    float a  = s->rot * 0.017453292519943;
    float rc = cosf(-a);
    float rs = sinf(-a);
    float rx = tx * rc - ty * rs;
    float ry = tx * rs + ty * rc;

    rx = rx + s->pos.x;
    ry = ry + s->pos.y;

    // compare with bounding box
    if (rx > s->pos.x - (s->size.x / 2) &&
        rx < s->pos.x + (s->size.x / 2) &&
        ry > s->pos.y - (s->size.y / 2) &&
        ry < s->pos.y + (s->size.y / 2)) {
        return(true);
    }
    return(false);
}

void SpriteRelease()
{
    glDeleteProgram(SpriteProgram);
    cnode_t* node = SpriteList->head;
    while (node != NULL) {
        Sprite* s = (Sprite*)node->data;
        free(s);
        node = node->next;
    }
    clistFreeList(&SpriteList);
}
