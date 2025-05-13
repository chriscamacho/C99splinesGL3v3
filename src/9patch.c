#define CGLM_USE_ANONYMOUS_STRUCT 1
#include <cglm/struct/vec2.h>
#include <cglm/struct/vec3.h>
#include <cglm/struct/vec4.h>
#include <cglm/struct/affine.h>

#include "util.h"
#include "sprite.h"




float patchData[] = {
    -0.5f,  0.5f,       0,      0,
    -0.5f, -0.5f,       0,      .25,
     0.5f, -0.5f,       .25,    .25,
     
     0.5f, -0.5f,       .25,    .25,
     0.5f,  0.5f,       .25,    0,
    -0.5f,  0.5f,       0,      0
};


GLuint patchProgram;

GLuint patchProjL;
GLuint patchAtlasL;
GLuint patchTexL;
GLuint patchTintL;
GLuint patchDepthL;

GLuint patchSizeL;
    
GLuint patchPosL;
GLuint patchUvL;

GLuint PatchVAO;
GLuint PatchVBO;

SpriteSet* currentSpriteSet ;
    
void initpatch()
{
    patchProgram = createProgramGlsl("data/9patch.glsl", false, "330");
    glUseProgram(patchProgram);

    patchProjL  = glGetUniformLocation(patchProgram, "MP");
    patchAtlasL = glGetUniformLocation(patchProgram, "texture0");
    patchTexL   = glGetUniformLocation(patchProgram, "tex");
    patchTintL  = glGetUniformLocation(patchProgram, "in_tint");
    patchDepthL = glGetUniformLocation(patchProgram, "depth");
    glCheckError(__FILE__, __LINE__);
    patchSizeL = glGetUniformLocation(patchProgram, "sz");
    
    patchPosL  = glGetAttribLocation(patchProgram, "vPos");
    patchUvL  = glGetAttribLocation(patchProgram, "vUV");
    glCheckError(__FILE__, __LINE__);
    glGenVertexArrays(1, &PatchVAO);
    glBindVertexArray(PatchVAO);
    glGenBuffers(1, &PatchVBO);
    glCheckError(__FILE__, __LINE__);

    glCheckError(__FILE__, __LINE__);
    glEnableVertexAttribArray(patchPosL);
    glCheckError(__FILE__, __LINE__);
    glEnableVertexAttribArray(patchUvL);
    glCheckError(__FILE__, __LINE__);
    glVertexAttribPointer(patchPosL, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, 0);
    glVertexAttribPointer(patchUvL, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, (void*)(sizeof(float)*2));
    glBindBuffer(GL_ARRAY_BUFFER, PatchVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(patchData), patchData, GL_DYNAMIC_DRAW);

    glCheckError(__FILE__, __LINE__);
}

void use9patch(SpriteSet* ss, mat4s* p)
{
    currentSpriteSet = ss;
    glUseProgram(patchProgram);
    glEnableVertexAttribArray(patchPosL);
    glEnableVertexAttribArray(patchUvL);
    ss->SpriteProj = *p;
    //glUniformMatrix4fv(patchProjL, 1, GL_FALSE, (float*)&(ss->SpriteProj.raw));
    glActiveTexture(GL_TEXTURE0 + ss->textureUnit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, ss->SpriteTexture);
    glUniform1i(patchAtlasL, ss->textureUnit);
}

void draw9patch(SpriteSet* ss, vec2s pos, float r,vec2s size) {
    CGLM_ALIGN(16) mat4s m;
    //CGLM_ALIGN(8) vec3s p = {{ pos.x, pos.y, 0 }};
    CGLM_ALIGN(8) vec3s p = {{ 0, 0, 0 }};
    m = glms_translate_make(p);
    //m = glms_rotated_z(m, r);
    m = glms_mat4_mul(ss->SpriteProj, m);
    //glUniformMatrix4fv(patchProjL, 1, GL_FALSE, (const GLfloat*) m.raw);
    glUniformMatrix4fv(patchProjL, 1, GL_FALSE, (const GLfloat*) ss->SpriteProj.raw);
    glUniform2f(patchSizeL, size.x, size.y);
    glUniform1i(patchTexL, 3);
    glUniform1f(patchDepthL, 0.1);
    
  
    glBindVertexArray(PatchVAO);
    glBindBuffer(GL_ARRAY_BUFFER, PatchVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(patchData), &patchData[0], GL_DYNAMIC_DRAW);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);  
    glCheckError(__FILE__, __LINE__); 
}
