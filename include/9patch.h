#ifndef __9PATCH_H
#define __9PATCH_H

void initpatch();
void draw9patch(SpriteSet* ss, vec2s pos, float r,vec2s size);
void use9patch(SpriteSet* ss, mat4s* p);

#endif

