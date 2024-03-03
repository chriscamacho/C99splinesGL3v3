#include <stdio.h>
#include "util.h"
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


// Bo           buffer object
// data         address
// location     of the attribute
// itemSize     items per vertex
// totalSize    of data in bytes
GLuint initElementArrayBuffer(GLuint Bo, float* data, GLuint location,
                              GLuint itemSize, GLuint totalSize)
{
    GLuint buffer;

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Bo);
    glGenBuffers(1, &buffer);

    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    glBufferData(GL_ARRAY_BUFFER, totalSize, data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, itemSize, GL_FLOAT, GL_FALSE,
                          sizeof(float) * itemSize, 0);

    return(buffer);
}

// random float between start and start+range
float rnd(float start, float range)
{
    return(start + range * ((float)rand() / (float)RAND_MAX));
}

// creates a shader from text
static GLuint createShader(GLenum type, const char* text)
{
    GLuint  shader;
    GLint   ok;
    GLsizei length;
    char    log[1024];

    shader = glCreateShader(type);
    if (shader != 0) {
        glShaderSource(shader, 1, (const GLchar**)&text, NULL);
        glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
        if (ok != GL_TRUE) {
            fprintf(stderr, "ERROR: Failed to compile shader\n");
            glGetShaderInfoLog(shader, 1024, &length, log);
            fprintf(stderr, "ERROR: \n%s\n\n", log);
            glDeleteShader(shader);
            shader = 0;
        }
    }
    return(shader);
}

// this creates a shader but assumes 2 strings for the source
// a define for the shader type and then the combined shaders
static GLuint createShaderGlsl(GLenum type, const char** text)
{
    GLuint  shader;
    GLint   ok;
    GLsizei length;
    char    log[1024];

    shader = glCreateShader(type);
    if (shader != 0) {
        glShaderSource(shader, 2, text, NULL);
        glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
        if (ok != GL_TRUE) {
            fprintf(stderr, "ERROR: Failed to compile shader\n");
            glGetShaderInfoLog(shader, 1024, &length, log);
            fprintf(stderr, "ERROR: \n%s\n\n", log);
            glDeleteShader(shader);
            shader = 0;
        }
    }
    return(shader);
}

// DEPRICATED old code use createProgramGlsl
// given the text for a vertex and fragment shader creates a
// shader program
GLuint createShaderProgram(const char* vsText, const char* fsText)
{
    GLuint  program = 0, vertexShader = 0, fragmentShader = 0;
    GLint   ok;
    GLsizei length;
    char    log[1024];

    vertexShader   = createShader(GL_VERTEX_SHADER, vsText);
    fragmentShader = createShader(GL_FRAGMENT_SHADER, fsText);
    if (vertexShader != 0 && fragmentShader != 0) {
        program = glCreateProgram();
        if (program != 0) {
            glAttachShader(program, vertexShader);
            glAttachShader(program, fragmentShader);
            glLinkProgram(program);
            glGetProgramiv(program, GL_LINK_STATUS, &ok);
            if (ok != GL_TRUE) {
                glGetProgramInfoLog(program, 1024, &length, log);
                fprintf(stderr, "error linking shader program\n%s\n\n", log);
                glDeleteProgram(program);
                glDeleteShader(vertexShader);
                glDeleteShader(fragmentShader);
                program = 0;
            }
        }
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return(program);
}

char* readFile(const char* fname)
{
    FILE* file = fopen(fname, "rb");

    if (file == NULL) {
        printf("Failed to open file '%s'\n", fname);
        return(NULL);
    }

    fseek(file, 0L, SEEK_END);
    long  fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(sizeof(char) * (fileSize + 1));
    if (buffer == NULL) {
        fclose(file);
        printf("Failed to allocate memory for file '%s'\n", fname);
        return(NULL);
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead != fileSize) {
        fclose(file);
        free(buffer);
        printf("Failed to read file '%s'\n", fname);
        return(NULL);
    }

    buffer[fileSize] = '\0';

    fclose(file);
    return(buffer);
}

// create a shader program from a single glsl file
// if defs should define at least vert and frag shader
// and optionally a geom shader
GLuint createProgramGlsl(char* glslFile, bool hasGeom)
{
    // todo version as a param...
    const char* vDef = "#version 330\n#define VERTEX_SHADER\n\n";
    const char* gDef = "#version 330\n#define GEOMETRY_SHADER\n\n";
    const char* fDef = "#version 330\n#define FRAGMENT_SHADER\n\n";

    char*       glsl = NULL;

    glsl = readFile(glslFile);
    if (!glsl) {
        return(0);
    }

    const char* vSrc[] = { vDef, glsl };
    const char* gSrc[] = { gDef, glsl };
    const char* fSrc[] = { fDef, glsl };

    GLuint      vsh = 0, gsh = 0, fsh = 0;

    vsh = createShaderGlsl(GL_VERTEX_SHADER, vSrc);
    if (hasGeom) {
        gsh = createShaderGlsl(GL_GEOMETRY_SHADER, gSrc);
    }
    fsh = createShaderGlsl(GL_FRAGMENT_SHADER, fSrc);

    if (!vsh || !fsh || (!gsh && hasGeom)) {
        if (glsl) {
            free(glsl);
        }
        return(0); // shader log will provide feedback
    }

    GLint  ok;

    GLuint program = glCreateProgram();
    if (program != 0) {
        glAttachShader(program, vsh);
        if (hasGeom) {
            glAttachShader(program, gsh);
        }
        glAttachShader(program, fsh);
        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &ok);
        if (ok != GL_TRUE) {
            char  log[1024];
            GLint length;
            glGetProgramInfoLog(program, 1023, &length, log);
            fprintf(stderr, "error linking shader program\n%s\n\n", log);
            glDeleteProgram(program);
            glDeleteShader(vsh);
            glDeleteShader(fsh);
            if (hasGeom) {
                glDeleteShader(gsh);
            }
            if (glsl) {
                free(glsl);
            }
            program = 0;
        }
    }

    glDeleteShader(vsh);
    glDeleteShader(fsh);
    if (hasGeom) {
        glDeleteShader(gsh);
    }
    free(glsl);
    return(program);
}

// turn a GL error code into something meaningful
struct token_string
{
    GLuint      Token;
    const char* String;
};

static const struct token_string Errors[] =
{
    { GL_NO_ERROR,                          "no error"                      },
    { GL_INVALID_ENUM,                      "invalid enumerant"             },
    { GL_INVALID_VALUE,                     "invalid value"                 },
    { GL_INVALID_OPERATION,                 "invalid operation"             },
    { GL_OUT_OF_MEMORY,                     "out of memory"                 },
#ifdef GL_EXT_framebuffer_object
    { GL_INVALID_FRAMEBUFFER_OPERATION_EXT, "invalid framebuffer operation" },
#endif
    { ~0,                                   NULL                            } /* end of list indicator */
};

static const GLubyte* glErrorString(GLenum errorCode)
{
    int i;

    for (i = 0; Errors[i].String; i++)
    {
        if (Errors[i].Token == errorCode) {
            return((const GLubyte*)Errors[i].String);
        }
    }

    return((const GLubyte*)0);
}

// if there is an error code in effect, turn it into a meaningful
// message should be called with __FILE__ and __LINE__ macros
void glCheckError(const char* f, int l)
{
    int e = glGetError();

    if (e == GL_NO_ERROR) {
        return;
    }
    fprintf(stderr, "%s line %i: %s\n", f, l, glErrorString(e));
    exit(-e);
}

// returns a texture handle from an image filename
GLuint loadImage(const char* filename)
{
    int            width, height, bpp;
    GLuint         texture;
    unsigned char* rgb = stbi_load(filename, &width, &height, &bpp, 4);

    // rgb is now 4 bytes per pixel, width*height*size(4). Or NULL if load failed.

    if (rgb == 0) {
        return(0);
    }

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgb);

    stbi_image_free(rgb);
    return(texture);
}

GLuint loadTextureAtlas(const char* fname, GLuint x, GLuint y, GLuint l)
{
    stbi_set_flip_vertically_on_load(true);
    // TODO parameters for size and number...
    int            width, height, bpp;
    GLuint         texture;
    unsigned char* rgb = stbi_load(fname, &width, &height, &bpp, 4);
    // rgb is now 4 bytes per pixel, width*height*size(4). Or NULL if load failed.
    if (rgb == 0) {
        return(0);
    }

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, x, y, l, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgb);

    stbi_image_free(rgb);

    return(texture);
}
