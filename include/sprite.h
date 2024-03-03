#ifndef __SPRITE_H
#define __SPRITE_H

#include "gl_core_3_3.h"
#include <cglm/struct/vec2.h>
#include "clist.h"

typedef struct _SpriteSet
{
    clist_t* SpriteList;
    GLuint   textureUnit;
    GLuint   SpriteTexture;
    CGLM_ALIGN(16) mat4s SpriteProj;
} SpriteSet;

typedef struct _Sprite
{
    vec2s  pos;
    vec2s  dragOff;
    bool   dragging;
    bool   draggable;
    vec2s  size;
    float  depth;
    float  rot;
    GLuint tex;
    CGLM_ALIGN(16) vec4s tint;
    CGLM_ALIGN(16) vec4s otint;
} Sprite;

void initSprites();
void renderSprite(Sprite* s);
void setSpritePerspective(mat4s* p);
bool SpriteInBounds(Sprite* s, float x, float y);
Sprite* newSprite(SpriteSet* ss, vec4s area);
void SpriteRelease();
void SpriteRenderAll();

SpriteSet* createSpriteSet(const char* fileName, int tu, int width, int height, int n);
void spriteSetRelease(SpriteSet* ss);
void useSpriteSet(SpriteSet* ss, mat4s* p);

#endif
