#include <cglm/struct/vec2.h>

#include "gl_core_3_3.h"
#include "util.h"
#include "sprite.h"  // splines dependency for control points
#include "spline.h"
#include "clist.h"

#include <stdio.h>

#define STEPS        64
#define BUFF_SIZE    (STEPS + 2) * 2 * sizeof(float)

static float  SplineData[BUFF_SIZE / sizeof(float)];

static GLuint SplineTintL  = 0;
static GLuint SplineAaL    = 0;
static GLuint SplineWidthL = 0;
static GLuint SplineMiterL = 0;
static GLuint SplineProjL  = 0;

static GLuint SplinePosL = 0;

static GLuint SplineProgram = 0;

static GLuint SplineVAO = 0;
static GLuint SplineVBO = 0;

clist_t*      SplineList;
CGLM_ALIGN(16) mat4s SplineProj;

void initSpline()
{
    SplineList    = clistCreateList();
    SplineProgram = createProgramGlsl("data/rich_lines.glsl", true);
    glUseProgram(SplineProgram);

    SplineTintL  = glGetUniformLocation(SplineProgram, "color");
    SplineAaL    = glGetUniformLocation(SplineProgram, "antialias");
    SplineWidthL = glGetUniformLocation(SplineProgram, "linewidth");
    SplineMiterL = glGetUniformLocation(SplineProgram, "miter_limit");
    SplineProjL  = glGetUniformLocation(SplineProgram, "projection");

    SplinePosL = glGetAttribLocation(SplineProgram, "position");

    glUniform1f(SplineMiterL, -1); // never (don't!) changes...
    glUniform1f(SplineAaL, 1);     // has as property of Spline ???

    glGenVertexArrays(1, &SplineVAO);
    glBindVertexArray(SplineVAO);
    glGenBuffers(1, &SplineVBO);

    glBindBuffer(GL_ARRAY_BUFFER, SplineVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SplineData), SplineData, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(SplinePosL);
    glVertexAttribPointer(SplinePosL, 2, GL_FLOAT, GL_FALSE, 8, 0);

    glCheckError(__FILE__, __LINE__);
}

Spline* newSpline(vec4s area)
{
    Spline* spline = calloc(1, sizeof(Spline));

    spline->start       = (vec2s) { { rnd(area.x, area.y), rnd(area.z, area.w) } };
    spline->cp1         = (vec2s) { { rnd(area.x, area.y), rnd(area.z, area.w) } };
    spline->cp2         = (vec2s) { { rnd(area.x, area.y), rnd(area.z, area.w) } };
    spline->end         = (vec2s) { { rnd(area.x, area.y), rnd(area.z, area.w) } };
    spline->width       = 8;
    spline->depth       = .99;
    spline->tint        = (vec4s) { { rnd(0.5, 0.5), rnd(0.5, 0.5), rnd(0.5, 0.5), 1 } };
    spline->startSprite = newSprite(area, 0);
    spline->cp1Sprite   = newSprite(area, 1);
    spline->cp2Sprite   = newSprite(area, 2);
    spline->endSprite   = newSprite(area, 3);
    clistAddNode(SplineList, spline);
    return(spline);
}

void updateSpline(Spline* s)
{
    s->start = s->startSprite->pos;
    s->cp1   = s->cp1Sprite->pos;
    s->cp2   = s->cp2Sprite->pos;
    s->end   = s->endSprite->pos;
}

void setSplinePerspective(mat4s* p)
{
    glUseProgram(SplineProgram);
    SplineProj = *p;
    glCheckError(__FILE__, __LINE__);
}

void renderSpline(Spline* s)
{
    glUseProgram(SplineProgram);
    SplineProj.raw[3][2] = s->depth;
    glUniformMatrix4fv(SplineProjL, 1, GL_FALSE, (float*)&SplineProj.raw);
    glUniform1f(SplineWidthL, s->width);
    glUniform4f(SplineTintL, s->tint.r, s->tint.g, s->tint.b, s->tint.a);

    float* data   = &SplineData[0];
    int    offset = 0;

    vec2s  points[4];
    float  t[STEPS];
    float  b0[STEPS], b1[STEPS], b2[STEPS], b3[STEPS];

    points[0] = s->start;
    points[1] = s->cp1;
    points[2] = s->cp2;
    points[3] = s->end;

    for (int i = 0; i < STEPS; i++)
    {
        t[i] = (float)i / (STEPS - 1);
        float one_minus_t = 1.0f - t[i];
        b0[i] = one_minus_t * one_minus_t * one_minus_t;
        b1[i] = 3 * t[i] * one_minus_t * one_minus_t;
        b2[i] = 3 * t[i] * t[i] * one_minus_t;
        b3[i] = t[i] * t[i] * t[i];
    }
    data[offset++] = s->start.x;
    data[offset++] = s->start.y;
    for (int i = 0; i < STEPS; i++)
    {
        vec2s point = (vec2s) { { 0, 0 } };          // Initialize point to zero vector
        for (int j = 0; j < 4; j++)
        {
            vec2s temp = (vec2s) { { 0, 0 } };          // Initialize temp to zero vector
            if (j == 0) {
                temp.x = b0[i];
                temp.y = b0[i];
            } else if (j == 1) {
                temp.x = b1[i];
                temp.y = b1[i];
            } else if (j == 2) {
                temp.x = b2[i];
                temp.y = b2[i];
            } else if (j == 3) {
                temp.x = b3[i];
                temp.y = b3[i];
            }
            // Element-wise multiplication find cgml struct function TODO
            temp.x *= points[j].x;
            temp.y *= points[j].y;

            point = glms_vec2_add(point, temp);
        }
        // Assign the calculated point to the data array
        data[offset++] = point.x;
        data[offset++] = point.y;
    }
    data[offset++] = s->end.x;
    data[offset++] = s->end.y;

    glBindVertexArray(SplineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, SplineVBO);

    glEnableVertexAttribArray(SplinePosL);
    glVertexAttribPointer(SplinePosL, 2, GL_FLOAT, false, 8, 0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SplineData), &SplineData[0], GL_DYNAMIC_DRAW);

    glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, STEPS + 2); // Draw all points including start, end, and control points
    glCheckError(__FILE__, __LINE__);
}

void SplineRenderAll(mat4s* proj)
{
    setSplinePerspective(proj);
    cnode_t* node = SplineList->head;
    while (node != NULL) {
        Spline* s = (Spline*)node->data;
        updateSpline(s);
        renderSpline(s);
        node = node->next;
    }
}

void SplineRelease()
{
    glDeleteProgram(SplineProgram);
    // don't release the splines sprites as releasing the sprites will do that
    cnode_t* node = SplineList->head;
    while (node != NULL) {
        Spline* s = (Spline*)node->data;
        free(s);
        node = node->next;
    }
    clistFreeList(&SplineList);
}
