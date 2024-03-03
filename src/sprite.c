#include <cglm/struct/vec2.h>
#include <cglm/struct/vec4.h>

#include <stdio.h>
#include <math.h>

#include "gl_core_3_3.h"

#include "clist.h"
#include "util.h"
#include "sprite.h"

static GLuint     SpriteProgram = 0;
static GLuint     SpriteVAO     = 0;
static GLuint     SpriteVBO     = 0;

static GLuint     SpriteProjL  = 0;
static GLuint     SpriteSizeL  = 0;
static GLuint     SpriteAtlasL = 0;
static GLuint     SpritePosL   = 0;
static GLuint     SpriteRotL   = 0;
static GLuint     SpriteTexL   = 0;
static GLuint     SpriteTintL  = 0;
static GLuint     SpriteDepthL = 0;

static float      SpriteData[5 * sizeof(float)];
static SpriteSet* currentSpriteSet;

Sprite* newSprite(SpriteSet* ss, vec4s area)
{
    Sprite* s = calloc(1, sizeof(Sprite));

    s->pos.x = rnd(area.x, area.y);
    s->pos.y = rnd(area.z, area.w);

    s->size  = (vec2s) { { 32, 32 } };
    s->rot   = 0;
    s->depth = .6;
    s->tex   = 1;

    s->tint = (vec4s) { { 1, 1, 1, 1 } };
    s->otint = s->tint;

    s->dragOff   = (vec2s)GLMS_VEC2_ZERO_INIT;
    s->dragging  = false;
    s->draggable = true;
    clistAddNode(ss->SpriteList, s);
    return(s);
}

void initSprites()
{
    SpriteProgram = createProgramGlsl("data/sprite.glsl", true, "330");
    glUseProgram(SpriteProgram);

    SpriteProjL  = glGetUniformLocation(SpriteProgram, "projection");
    SpriteAtlasL = glGetUniformLocation(SpriteProgram, "texture0");
    SpriteTexL   = glGetUniformLocation(SpriteProgram, "tex");
    SpriteTintL  = glGetUniformLocation(SpriteProgram, "in_tint");
    SpriteDepthL = glGetUniformLocation(SpriteProgram, "depth");

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

    glCheckError(__FILE__, __LINE__);
}

/**
 * Creates a sprite set
 *
 * Sprite sets are a list of sprites and what texture is used to render
 * them you must call useSpriteSet before rendering sprites
 *
 * @param[in] fileName the image to use for the texture it must be width wide and height * n high
 * @param[in] tu texture unit to use
 * @param[in] width the width of 1 sprite
 * @param[in] height the height of 1 sprite
 * @param[in] n number of sprites in the texture
 * @param[out] SpriteSet*
 */
SpriteSet* createSpriteSet(const char* fileName, int tu, int width, int height, int n)
{
    SpriteSet* ss = calloc(1, sizeof(SpriteSet));

    ss->SpriteList = clistCreateList();
    glUseProgram(SpriteProgram);
    glCheckError(__FILE__, __LINE__);
    ss->SpriteTexture = loadTextureAtlas(fileName, width, height, n);
    glCheckError(__FILE__, __LINE__);
    ss->textureUnit = tu;
    glCheckError(__FILE__, __LINE__);
    glUniform1i(SpriteAtlasL, tu);
    glCheckError(__FILE__, __LINE__);
    glBindTexture(GL_TEXTURE_2D_ARRAY, ss->SpriteTexture);
    glCheckError(__FILE__, __LINE__);
    return(ss);
}

void spriteSetRelease(SpriteSet* ss)
{
    cnode_t* node = ss->SpriteList->head;

    while (node != NULL) {
        Sprite* s = (Sprite*)node->data;
        free(s);
        node = node->next;
    }
    clistFreeList(&ss->SpriteList);
    free(ss);
}

void useSpriteSet(SpriteSet* ss, mat4s* p)
{
    currentSpriteSet = ss;
    glUseProgram(SpriteProgram);
    glEnableVertexAttribArray(SpritePosL);
    glEnableVertexAttribArray(SpriteSizeL);
    glEnableVertexAttribArray(SpriteRotL);
    ss->SpriteProj = *p;
    glUniformMatrix4fv(SpriteProjL, 1, GL_FALSE, (float*)&(ss->SpriteProj.raw));
    glActiveTexture(GL_TEXTURE0 + ss->textureUnit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, ss->SpriteTexture);
    glUniform1i(SpriteAtlasL, ss->textureUnit);
}

static void derefAndDraw(cnode_t* node)
{
    Sprite* s = (Sprite*)(node->data);

    renderSprite(s);
}

// todo should there be a current SpriteSet so this doesn't need a param ?
void SpriteRenderAll()
{
    clistIterateForward(currentSpriteSet->SpriteList, derefAndDraw);
}

void renderSprite(Sprite* s)
{
    glUniform1i(SpriteTexL, s->tex);
    glUniform1f(SpriteDepthL, s->depth);
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

    glDrawArrays(GL_POINTS, 0, 1);
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
}
