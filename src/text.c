#include <stdio.h>
#include <string.h>

#define CGLM_USE_ANONYMOUS_STRUCT 1
#include <cglm/struct/vec2.h>
#include <cglm/struct/vec4.h>
#include <cglm/struct/mat4.h>
#include <cglm/struct/affine.h>
#include "sprite.h"

// this isn't on the global render list...
Sprite tr;

// allow different font textures, char size, and advance

void initText()
{
    // use a single sprite to render each character
    tr.size     = (vec2s) { { 12, 24 } };
    tr.rot      = 0;
    tr.tex      = 96;
    tr.tint     = (vec4s) { { 1, 1, 1, 1 } };
    tr.otint    = tr.tint;
    tr.dragOff  = (vec2s)GLMS_VEC2_ZERO_INIT;
    tr.dragging = false;
    tr.depth    = 0;
}

void renderText(const char* text, vec2s p, float d, float r, bool centre, SpriteSet* ss)
{
    CGLM_ALIGN(16) mat4s R  = glms_mat4_ucopy(ss->SpriteProj);
    CGLM_ALIGN(16) vec3s tv = (vec3s){ { p.x, p.y, 0 } };
    CGLM_ALIGN(16) mat4s T  = glms_mat4_ucopy(ss->SpriteProj);
    T = glms_translate(T, tv);
    T = glms_rotate_z(T, r);
    float l  = strlen(text) * 6;
    float ly = 0;
    if (!centre) {
        l  = -6;
        ly = 12;
    }
    tv = (vec3s){ { -l, ly, 0 } };
    T  = glms_translate(T, tv);

    useSpriteSet(ss, &T);
    tr.pos = GLMS_VEC2_ZERO;
    char c = text[0];
    int  i = 1;
    while (c) {
        tr.tex = (int)(c - 32);
        renderSprite(&tr);
        tr.pos.x += 12;
        c         = text[i++];
    }
    // restore sprite set matrix
    ss->SpriteProj = glms_mat4_ucopy(R);
}
