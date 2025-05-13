#include <stdio.h>
#include <stdbool.h>
#include <time.h> // for random seed

#include "gl_core_3_3.h"
#include <GLFW/glfw3.h>

// vec3: 8 byte vec4: 16 byte mat4: 16 byte versor: 16 byte

// CGLM_ALIGN(8) for vec3s
// CGLM_ALIGN(16) for vec4s and mat4s

#define CGLM_USE_ANONYMOUS_STRUCT 1
#include <cglm/struct/vec2.h>
#include <cglm/struct/vec3.h>
#include <cglm/struct/vec4.h>
#include <cglm/struct/affine.h>
#include <cglm/struct/cam.h>

#include "clist.h"
#include "util.h"
#include "sprite.h"
#include "spline.h"
#include "text.h"

#include "main.h"

#define Iwidth        960   // initial window size
#define Iheight       540

#define numSplines    4
#define winTitle      "C99 OpenGL splines"   // also used by fps

// TODO create a mouse struct in main.h
vec2s   mousePos     = GLMS_VEC2_ZERO_INIT;           // mouse, screen coords, screen centre is (0,0)
vec2s   lastMousePos = GLMS_VEC2_ZERO_INIT;
CGLM_ALIGN(8) vec3s pointerPos = GLMS_VEC3_ZERO_INIT; // unprojected "world" mouse coords
bool    dragging = false;
Sprite* mouseSprite;

bool    dumpSplines = false; // key event signal to main loop


// creates a combined matrix to provide zoom scale and projection
mat4s combinedMatrix(WinData* w)
{
    CGLM_ALIGN(16) mat4s S   = glms_scale_make(w->zoom);
    CGLM_ALIGN(16) mat4s T   = glms_translate_make(w->pan);
    CGLM_ALIGN(16) mat4s PS  = glms_mat4_mul(w->proj, S);
    CGLM_ALIGN(16) mat4s PST = glms_mat4_mul(PS, T);

    return(PST);
}

static void error_callback(int error, const char* description)
{
    printf("GLFW Error: %s\n\n", description);
}

// allows exit and also fullscreen toggle
static void key_callback(GLFWwindow* window, int key, int scancode,
                         int action, int mods)
{
    WinData* wd = glfwGetWindowUserPointer(window);

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        if (wd->fullscreen) {
            glfwSetWindowMonitor(window, 0,
                                 wd->mode->width / 4, wd->mode->height / 4,
                                 wd->mode->width / 2, wd->mode->height / 2, 0);
            wd->fullscreen = false;
        } else {
            glfwSetWindowMonitor(window, wd->monitor, 0, 0, wd->mode->width,
                                 wd->mode->height, wd->mode->refreshRate);
            wd->fullscreen = true;
        }
    }

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        dumpSplines = true;
    }
}

// recalculate the projection matrix when the window changes size
static void windowSizeCallback(GLFWwindow* window, int w, int h)
{
    WinData* wd = glfwGetWindowUserPointer(window);

    wd->winSize = glms_vec2_make((vec2) { w, h });
    vec2s    ws = wd->winSize;
    wd->proj = glms_ortho(-ws.x / 2, ws.x / 2, -ws.y / 2, ws.y / 2, 0, 1.0);
    glViewport(0, 0, ws.x, ws.y);
}

// used to be loads in here moved to main loop update...
static void mousePosCallback(GLFWwindow* window, double xpos, double ypos)
{
}

// zoom (scale screen) with scroll wheel
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    WinData* wd = glfwGetWindowUserPointer(window);

    yoffset    /= 12.5;
    wd->zoom.x += yoffset;
    if (wd->zoom.x < 0.25) {
        wd->zoom.x = 0.25;
    }
    if (wd->zoom.x > 5) {
        wd->zoom.x = 5;
    }
    wd->zoom.y = wd->zoom.x;

    //printf("zoom %.2f\n", wd->zoom.x);
}

// ---------------------------------------------------------------------
//                          entry point
// ---------------------------------------------------------------------
int main()
{
    srand(time(NULL));

    WinData winData; // stuff needed callbacks and main code

    winData.proj       = GLMS_MAT4_IDENTITY;
    winData.zoom       = (vec3s){ { 1.0, 1.0, 1.0 } };
    winData.fullscreen = false;

    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        printf("Can't initialise GLFW\n");
        return(1);
    }

    // tell GLFW what window properties / GL context we want
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_DEPTH_BITS, 16);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

    window = glfwCreateWindow(Iwidth, Iheight, winTitle, NULL, NULL);
    if (!window) {
        printf("Can't create a window\n");
        glfwTerminate();
        return(1);
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, &winData);

    // comment this when debugging cursor and projection issues!
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    glfwSetWindowAspectRatio(window, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, 1); // polls show us key states
    glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
    glfwSwapInterval(1);

    // get the monitor and mode for fullscreen/window toggle
    winData.monitor = glfwGetPrimaryMonitor();
    winData.mode    = glfwGetVideoMode(winData.monitor);

    // setup the callback and the GL context
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetWindowSizeCallback(window, windowSizeCallback);
    glfwSetCursorPosCallback(window, mousePosCallback);

    // load the OpenGL functions
    if (ogl_LoadFunctions() == ogl_LOAD_FAILED) {
        printf("failed to load GL function pointers");
        glfwTerminate();
        return(1);
    }

    glCheckError(__FILE__, __LINE__);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearDepth(1.0);
    glDepthRange(0.0, 1.0);
    glDisable(GL_CULL_FACE);
    glClearColor(.2, .2, .2, 1.f);

    // manually call to ensure proj matrix set
    windowSizeCallback(window, Iwidth, Iheight);


    initSpline();
    glCheckError(__FILE__, __LINE__);

    initSprites(); // set up sprite renderer
    glCheckError(__FILE__, __LINE__);

    initText();  // set up the text renderer
    glCheckError(__FILE__, __LINE__);

    SpriteSet* fontSet = createSpriteSet("data/font.png", 0, 64, 64, 95);
    glCheckError(__FILE__, __LINE__);
    SpriteSet* spriteSet = createSpriteSet("data/atlas.png", 1, 256, 256, 6);

    float      hw = winData.winSize.x / 2.0;
    float      hh = winData.winSize.y / 2.0;
    winData.pan = (vec3s) { { 0, 0, 0.0 } };

    mouseSprite            = newSprite(spriteSet, (vec4s){ { -1, 2, -1, 2 } });
    mouseSprite->tint      = (vec4s) { { 0.75, 0.2, 1, 1 } };
    mouseSprite->tex       = 0;
    mouseSprite->rot       = 0;
    mouseSprite->size.x    = 64;
    mouseSprite->size.y    = 64;
    mouseSprite->draggable = false;
    mouseSprite->depth     = 0;

    glCheckError(__FILE__, __LINE__);

    float start[] =
    {
        -168.2221, 104.9158, -446.8788,  238.5632,  -38.4472,  238.5745, -310.8877, 104.5538,
        -140.6923, -97.9892, -295.1754,   11.9682, -289.7881, -229.4157, -448.7034, -99.8921,
        103.7043,  181.3177,  349.1771,   22.4101,  209.0544,   21.2095,  433.5792, 196.5060,
        101.9727,  -48.8820,  208.1340, -204.8342,  361.7113, -204.4507,  441.0488, -44.0520,
    };
    int   ix = 0;
    
    vec4s sc[4];
    sc[0] = (vec4s) { { 1, 0, 0, 1 } };
    sc[1] = (vec4s) { { 1, 0, 1, 1 } };
    sc[2] = (vec4s) { { 0, 1, 1, 1 } };
    sc[3] = (vec4s) { { 0, 1, 0, 1 } };

    for (int i = 0; i < numSplines; i++)
    {
        CGLM_ALIGN(16) vec4s area;
        area = (vec4s) { { -hw / 2 + 128, winData.winSize.x - 256,
                           -hh / 2 + 128, winData.winSize.y - 256 } };
        Spline* s = newSpline(spriteSet, area);
        s->startSprite->pos.x = start[ix++];
        s->startSprite->pos.y = start[ix++];
        s->startSprite->tint = sc[0];
        s->startSprite->otint = sc[0];
        s->cp1Sprite->pos.x   = start[ix++];
        s->cp1Sprite->pos.y   = start[ix++];
        s->cp1Sprite->tint = sc[1];
        s->cp1Sprite->otint = sc[1];
        s->cp2Sprite->pos.x   = start[ix++];
        s->cp2Sprite->pos.y   = start[ix++];
        s->cp2Sprite->tint = sc[2];
        s->cp2Sprite->otint = sc[2];
        s->endSprite->pos.x   = start[ix++];
        s->endSprite->pos.y   = start[ix++];
        s->endSprite->tint = sc[3];
        s->endSprite->otint = sc[3];
    }

    for (int i = 0; i < 8; i++)
    {
        CGLM_ALIGN(16) vec4s area;
        area = (vec4s) { { -hw + 128, winData.winSize.x - 256,
                           -hh + 128 + hh * 2, winData.winSize.y - 256 } };
        Sprite* s = newSprite(spriteSet, area);
        s->tint  = (vec4s){ { rnd(0.5, 0.5), rnd(0.5, 0.5), rnd(0.5, 0.5), 1 } };
        s->otint = s->tint;
        s->tex   = rnd(2, 4);
        s->size  = (vec2s){ { 64, 64 } };
    }

    // -------------------------------------------------------------------
    //                          main loop
    // -------------------------------------------------------------------
    double lastTime = glfwGetTime();
    double fpsCount = 0;
    float  FPS      = 0;
    float  fr       = 0; // rotation of text
    int    frames   = 0;

    while (!glfwWindowShouldClose(window)) {
        frames++;
        // -------  FPS  ------ (TODO make getFps in utils)
        double thisTime = glfwGetTime(); // Get the current timestamp
        fpsCount++;
        if ((thisTime - lastTime) >= 0.166666666) {
            char title[80];
            sprintf(title, "%s (%.02f FPS)", winTitle, fpsCount * 4.0);
            glfwSetWindowTitle(window, title); // The title
            FPS       = fpsCount * 4.0;
            fpsCount  = 0;
            lastTime += 0.25;
        }

        //--------------------------------------------------------------
        //                    Update
        //--------------------------------------------------------------
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // mouse pointer axis, centre of screen
        mousePos.x = xpos - (winData.winSize.x / 2);
        mousePos.y = -ypos + (winData.winSize.y / 2);

        // update sprite pointer
        mouseSprite->pos.x = pointerPos.x + (16 * (1.0 / winData.zoom.x));
        mouseSprite->pos.y = pointerPos.y - (16 * (1.0 / winData.zoom.y));

        // ensure mouse sprite stays unzoomed
        mouseSprite->size.x = 32 * (1.0 / winData.zoom.x);
        mouseSprite->size.y = mouseSprite->size.x;

        // middle mouse pans screen
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
            winData.pan.x = winData.pan.x - ((lastMousePos.x - mousePos.x) *
                                             (1.0 / winData.zoom.x));
            winData.pan.y = winData.pan.y - ((lastMousePos.y - mousePos.y) *
                                             (1.0 / winData.zoom.x));
        }

        // sprite dragging
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            if (!dragging) { // offset captured only at drag start
                cnode_t* node = spriteSet->SpriteList->head;
                while (node != NULL) {
                    Sprite* s = (Sprite*)node->data;
                    if (s->draggable) {
                        if (SpriteInBounds(s, pointerPos.x, pointerPos.y)) {
                            s->dragOff.x = s->pos.x - pointerPos.x;
                            s->dragOff.y = s->pos.y - pointerPos.y;
                            s->dragging  = true;
                        }
                    }
                    node = node->next;
                }
                dragging = true;
            } else { // actually dragging sprites
                cnode_t* node = spriteSet->SpriteList->head;
                while (node != NULL) {
                    Sprite* s = (Sprite*)node->data;
                    if (s->dragging) {
                        vec2s pp = (vec2s) { { pointerPos.x, pointerPos.y } };
                        s->pos = glms_vec2_add(pp, s->dragOff);
                    }
                    node = node->next;
                }
            }
        } else { // button up, is still dragging reset all drag states
            if (dragging) {
                cnode_t* node = spriteSet->SpriteList->head;
                while (node != NULL) {
                    Sprite* s = (Sprite*)node->data;
                    s->dragging = false;
                    node        = node->next;
                }
                dragging = false;
            }
        }

        CGLM_ALIGN(16) mat4s PST = combinedMatrix(&winData);

        // unproject then account for pan and zoom on mouse
        CGLM_ALIGN(16) mat4s inv = glms_mat4_inv(PST);
        // unproject mouse allowing for pan and zoom
        CGLM_ALIGN(16) vec4s mp;
        mp.x = mousePos.x - (winData.pan.x * winData.zoom.x);
        mp.y = mousePos.y - (winData.pan.y * winData.zoom.y);
        mp.z = 0;
        mp.w = 0;

        CGLM_ALIGN(16) vec4s mpout;
        mpout        = glms_mat4_mulv(inv, mp);
        pointerPos.x = mpout.x / (winData.winSize.x / 2);
        pointerPos.y = mpout.y / (winData.winSize.y / 2);

        lastMousePos.x = mousePos.x;
        lastMousePos.y = mousePos.y;

        if (dumpSplines) {
            cnode_t* node = SplineList->head;
            while (node != NULL) {
                Spline* s = (Spline*)node->data;

                printf("%.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f,\n",
                       s->startSprite->pos.x, s->startSprite->pos.y,
                       s->cp1Sprite->pos.x, s->cp1Sprite->pos.y,
                       s->cp2Sprite->pos.x, s->cp2Sprite->pos.y,
                       s->endSprite->pos.x, s->endSprite->pos.y);
                node = node->next;
            }
            dumpSplines = false;
        }

        cnode_t* node = spriteSet->SpriteList->head;
        while (node != NULL) {
            Sprite* s = (Sprite*)node->data;
            if (s->draggable) {
                if (SpriteInBounds(s, pointerPos.x, pointerPos.y)) {
                    s->tint.r = s->otint.r * .6;
                    s->tint.g = s->otint.g * .6;
                    s->tint.b = s->otint.b * .6;
                } else {
                    s->tint.r = s->otint.r;
                    s->tint.g = s->otint.g;
                    s->tint.b = s->otint.b;
                }
            }
            node = node->next;
        }

        //--------------------------------------------------------------
        //                    Render
        //--------------------------------------------------------------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        char fpsStr[80] = { 0 };
        fr += 0.01;
        sprintf(fpsStr, "FPS: %03.02f", FPS);
        useSpriteSet(fontSet, &PST);
        renderText(fpsStr, (vec2s){ { 0, 0 } }, .8, fr, true, fontSet);

        useSpriteSet(fontSet, &winData.proj);
        renderText(fpsStr, (vec2s){ { -(winData.winSize.x / 2) + 2, (winData.winSize.y / 2) - 24 } }, .8, 0, false, fontSet);
        sprintf(fpsStr, "Zoom: %.2f Frames: %i", winData.zoom.x, frames);
        renderText(fpsStr, (vec2s){ { -(winData.winSize.x / 2) + 2, (-winData.winSize.y / 2) } }, .8, 0, false, fontSet);

        sprintf(fpsStr, "Pan up to see sprites");
        useSpriteSet(fontSet, &PST);
        renderText(fpsStr, (vec2s){ { 0, (winData.winSize.y / 2) - 128 } }, .8, 0, true, fontSet);

        SplineRenderAll(&PST);
        glCheckError(__FILE__, __LINE__);
        useSpriteSet(spriteSet, &PST);
        SpriteRenderAll();
        glCheckError(__FILE__, __LINE__);

        glfwSwapBuffers(window);
        glfwPollEvents();
        //glfwSetWindowShouldClose(window, GLFW_TRUE);  // for debugging run just a single frame
    }

    // just to catch anything missed in main loop ...
    glCheckError(__FILE__, __LINE__);

    SpriteRelease(); // free sprites first.
    SplineRelease();
    spriteSetRelease(fontSet);
    spriteSetRelease(spriteSet);

    glCheckError(__FILE__, __LINE__);

    glfwDestroyWindow(window);
    glfwTerminate();
    return(0);
}
