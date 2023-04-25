#ifndef GL_DEFINITIONS_H
#define GL_DEFINITIONS_H

#define GL_COLOR_BUFFER_BIT               0x00004000

#define GL_ARRAY_BUFFER                   0x8892
#define GL_STATIC_DRAW                    0x88E4

#define GL_VERSION                        0x1F02;

typedef int           GLsizei;
typedef unsigned int  GLenum;
typedef unsigned int  GLuint;
typedef unsigned char GLubyte;
typedef unsigned int  GLbitfield;
typedef float         GLclampf;
typedef size_t        GLsizeiptr;

// GL function typedefs
#define X(N,R,P) typedef R GL_##N##Func P;
#include "gl/gl_funcs.inc"
#include "gl/gl_ext_funcs.inc"
#undef X

// global GL function pointers
#define X(N,R,P) global GL_##N##Func *N;
#include "gl/gl_funcs.inc"
#include "gl/gl_ext_funcs.inc"
#undef X

#endif // GL_DEFINITIONS_H
