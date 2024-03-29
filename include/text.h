#ifndef __TEXT_H
#define __TEXT_H

#include <cglm/struct/vec2.h>

void renderText(const char* text, vec2s p, float d, float r, bool centre, SpriteSet* ss);
void initText();

#endif
