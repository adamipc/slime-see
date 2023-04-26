#ifndef GL_H
#define GL_H

function String8 gl_init();
function String8 gl_ext_init(W32_wglGetProcAddressFunc v_wglGetProcAddress);
function GLuint  gl_create_shader_program(M_Arena *arena, String8 vss, String8 fss, String8List feedback_varyings);

#endif // GL_H
