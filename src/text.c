#include <stdio.h>
#include <string.h>

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
    tr.size     = (vec2s) { { 12, 24 } };
    tr.rot      = 0;
    tr.tex      = 96;
    tr.tint     = (vec4s) { { 1, 1, 1, 1 } };
    tr.otint    = tr.tint;
    tr.dragOff  = (vec2s)GLMS_VEC2_ZERO_INIT;
    tr.dragging = false;
}

void renderText(const char* text, vec2s p, float r, mat4s M)
{
    CGLM_ALIGN(16) vec3s tv = (vec3s){ { p.x, p.y, 0 } };
    CGLM_ALIGN(16) mat4s T  = glms_mat4_ucopy(M);
    T = glms_translate(T, tv);
    T = glms_rotate_z(T, r);
    float l = strlen(text) * 6;
    tv = (vec3s){ { -l, 0, 0 } };
    T  = glms_translate(T, tv);

    setSpritePerspective((float*)T.raw);

    tr.pos = GLMS_VEC2_ZERO;
    char c = text[0];
    int  i = 0;
    while (c) {
        tr.tex = (int)(c - 32);
        renderSprite(tr);
        tr.pos.x += 12;
        c         = text[i++];
    }
}
