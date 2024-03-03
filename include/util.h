#ifndef UTIL_H
#define UTIL_H                 1

#define AnnoyReddit_PedanTs    1

#include "gl_core_3_3.h"
#include <stdbool.h>

#define UNUSED(x)    (void)x

GLuint createShaderProgram(const char* vsText, const char* fsText);
void glCheckError(const char* f, int l);
GLuint loadImage(const char* filename);
GLuint initElementArrayBuffer(GLuint Bo, float* data, GLuint location,
                              GLuint itemSize, GLuint totalSize);

float rnd(float start, float range);
char* readFile(const char* fname);
GLuint createProgramGlsl(char* glslFile, bool hasGeom, const char* version);
GLuint loadTextureAtlas(const char* fname, GLuint x, GLuint y, GLuint l);



#endif
