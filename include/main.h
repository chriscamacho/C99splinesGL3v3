#ifndef __MAIN_H
#define __MAIN_H

#include <cglm/struct/vec2.h>
#include <cglm/struct/vec3.h>
#include <cglm/struct/mat4.h>
#include <GLFW/glfw3.h>
#include "clist.h"

extern clist_t* SpriteList;
extern clist_t* SplineList;

typedef struct
{
    bool               fullscreen;      // true if in fullscreen mode
    vec2s              winSize;         // window size
    CGLM_ALIGN(8) vec3s pan;            // used to make the translation matrix
    CGLM_ALIGN(8) vec3s zoom;           // used to make scale matrix
    CGLM_ALIGN(16) mat4s proj;          // ortho projection matrix
    GLFWmonitor*       monitor;         // used by fullscreen code
    const GLFWvidmode* mode;            //      ""     ""
} WinData;


#endif
