#ifndef __SPRITE_H
#define __SPRITE_H

#include "gl_core_3_3.h"
#include <cglm/struct/vec2.h>

typedef struct _Sprite
{
    vec2s  pos;
    vec2s  dragOff;
    bool   dragging;
    vec2s  size;
    float  rot;
    GLuint tex;
    CGLM_ALIGN(16) vec4s tint;
    CGLM_ALIGN(16) vec4s otint;
} Sprite;

void initSprites();
void renderSprite(Sprite s);
void setSpritePerspective(float* p);
bool SpriteInBounds(Sprite s, float x, float y);
Sprite* newSprite(vec4s area, int cpType);
void SpriteRelease();
void SpriteRenderAll(float* proj);

#endif
