#ifndef GL_DEFINITIONS_H
#define GL_DEFINITIONS_H


#define GL_FALSE                          0
#define GL_NO_ERROR                       0

#define GL_TRIANGLES                      0x0004

#define GL_FRONT_AND_BACK                 0x0408

#define GL_FLOAT                          0x1406
#define GL_UNSIGNED_INT                   0x1405

#define GL_LINE                           0x1B01

#define GL_VERSION                        0x1F02

#define GL_COLOR_BUFFER_BIT               0x00004000

#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893

#define GL_STATIC_DRAW                    0x88E4

#define GL_DYNAMIC_COPY                   0x88EA

#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31

#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82

#define GL_SEPARATE_ATTRIBS               0x8C8D

typedef int           GLsizei;
typedef int           GLint;
typedef bool          GLboolean;
typedef char          GLchar;
typedef float         GLfloat;
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
