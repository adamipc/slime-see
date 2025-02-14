#ifndef GL_DEFINITIONS_H
#define GL_DEFINITIONS_H


#define GL_FALSE                          0
#define GL_NO_ERROR                       0

#define GL_ZERO                           0x0000
#define GL_ONE                            0x0001

#define GL_POINTS                         0x0000
#define GL_TRIANGLES                      0x0004
#define GL_TRIANGLE_FAN                   0x0006

#define GL_GREATER                        0x0204

#define GL_SRC_COLOR                      0x0300
#define GL_ONE_MINUS_SRC_COLOR            0x0301
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_DST_ALPHA                      0x0304
#define GL_ONE_MINUS_DST_ALPHA            0x0305
#define GL_DST_COLOR                      0x0306
#define GL_ONE_MINUS_DST_COLOR            0x0307
#define GL_SRC_ALPHA_SATURATE             0x0308

#define GL_CONSTANT_COLOR                 0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR       0x8002
#define GL_CONSTANT_ALPHA                 0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA       0x8004

#define GL_SRC1_COLOR                     0x88F9
#define GL_ONE_MINUS_SRC1_COLOR           0x88FA
#define GL_SRC1_ALPHA                     0x8589
#define GL_ONE_MINUS_SRC1_ALPHA           0x88FB

#define GL_FRONT_AND_BACK                 0x0408

#define GL_ALPHA_TEST                     0x0BC0
#define GL_BLEND                          0x0BE2

#define GL_TEXTURE_2D                     0x0DE1

#define GL_UNSIGNED_BYTE                  0x1401
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406

#define GL_LINE                           0x1B01

#define GL_RED                            0x1903
#define GL_GREEN                          0x1904
#define GL_BLUE                           0x1905
#define GL_ALPHA                          0x1906
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908
#define GL_LUMINANCE                      0x1909

#define GL_VERSION                        0x1F02

#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601

#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803

#define GL_COLOR_BUFFER_BIT               0x00004000

#define GL_CLAMP_TO_EDGE                  0x812F

#define GL_DEPTH_COMPONENT16              0x81A5
#define GL_DEPTH_COMPONENT24              0x81A6

#define GL_DEBUG_TYPE_ERROR               0x824C

#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1

#define GL_PROGRAM_POINT_SIZE             0x8642

#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893

#define GL_STATIC_DRAW                    0x88E4

#define GL_DYNAMIC_COPY                   0x88EA

#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31

#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82

#define GL_SEPARATE_ATTRIBS               0x8C8D
#define GL_TRANSFORM_FEEDBACK_BUFFER      0x8C8E

#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE           0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME           0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL         0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE 0x8CD3
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5

#define GL_COLOR_ATTACHMENT0              0x8CE0

#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_STENCIL_ATTACHMENT             0x8D20
#define GL_FRAMEBUFFER                    0x8D40
#define GL_RENDERBUFFER                   0x8D41

#define GL_TRANSFORM_FEEDBACK             0x8E22

#define GL_DEBUG_OUTPUT                   0x92E0



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

typedef void (APIENTRY  *GLDEBUGPROC)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam);

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
