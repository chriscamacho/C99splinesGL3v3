#ifndef __TEXT_H
#define __TEXT_H

#include <cglm/struct/vec2.h>

void renderText(const char* text, vec2s p, float r, bool centre, mat4s M);
void initText();

#endif
