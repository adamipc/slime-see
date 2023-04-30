#ifndef WIN32_WGL_H
#define WIN32_WGL_H

#include "win32/win32_wgl_definitions.h"

struct W32_OpenGLWindow {
  HWND window;
  HGLRC glrc;
};

function String8 win32_wgl_init(HINSTANCE Instance);
function W32_OpenGLWindow win32_create_opengl_window(HINSTANCE Instance, WNDPROC window_proc, int width, int height);
function String8 win32_wgl_ext_init(W32_wglGetProcAddressFunc v_wglGetProcAddress);

#endif WIN32_WGL_H
