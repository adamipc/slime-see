#include "gl/gl.h"

function String8
gl_ext_init(W32_wglGetProcAddressFunc v_wglGetProcAddress) {
  String8 error = {};
# define WGL_GET_PROC_ADDRESS(v,n) (*(PROC*)(&(v))) = v_wglGetProcAddress((n))
# define X(N,R,P) if (!error) {\
  WGL_GET_PROC_ADDRESS(N, #N);\
  if (N == 0) {\
    error = str8_lit("could not load " #N);\
  }\
}
# include "gl/gl_ext_funcs.inc"
# undef X

  return error;
}

function String8
gl_init() {
  String8 error = {};
  HINSTANCE opengl_module = LoadLibraryA("opengl32.dll");

  if (opengl_module == 0) {
    error = str8_lit("failed to initialize opengl32.dll");
  }

# define X(N,R,P) if (!error) {\
    GET_PROC_ADDRESS(N, opengl_module, #N);\
    if (N == 0) {\
      error = str8_lit("could not load " #N);\
    }\
  }
# include "gl/gl_funcs.inc"
# undef X

  return error;
}

