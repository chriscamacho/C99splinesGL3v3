#ifndef __SPLINE_H
#define __SPLINE_H

#include <cglm/struct/vec2.h>
#include <cglm/struct/vec4.h>
#include "sprite.h"

typedef struct
{
    vec2s   start;
    vec2s   end;
    vec2s   cp1;
    vec2s   cp2;
    float   width;
    float   depth;
    CGLM_ALIGN(16) vec4s tint;
    Sprite* startSprite;
    Sprite* cp1Sprite;
    Sprite* cp2Sprite;
    Sprite* endSprite;
} Spline;

void initSpline();
void setSplinePerspective(mat4s* p);
void renderSpline(Spline* s);
Spline* newSpline(SpriteSet* ss, vec4s area);
void updateSpline(Spline* s);
void SplineRelease();
void SplineRenderAll(mat4s* p);

#endif
