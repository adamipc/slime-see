#include "win32/win32_wgl.h"

LRESULT CALLBACK bootstrap_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message)
  {
    case WM_CREATE: {
    } break;
    default: {
      return DefWindowProc(hWnd, message, wParam, lParam);
    } break;
  }

  return 0;
}

LRESULT CALLBACK graphics_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message)
  {
    case WM_CREATE: {
    } break;
    default: {
      return DefWindowProc(hWnd, message, wParam, lParam);
    } break;
  }

  return 0;
}

function W32_OpenGLWindow
win32_create_opengl_window(HINSTANCE Instance) {
  String8 error = {};

  HWND graphics_window;
  {
    WNDCLASSA window_class = {};
    window_class.lpfnWndProc = graphics_wndproc;
    window_class.hInstance = Instance;
    window_class.lpszClassName = "graphics_window";

    if (!RegisterClass(&window_class)) {
      error = str8_lit("failed to register window class");
      goto done;
    }

    HWND hWnd = CreateWindowA(window_class.lpszClassName,
                  "My Window",
                  WS_TILEDWINDOW,
                  CW_USEDEFAULT,
                  CW_USEDEFAULT,
                  CW_USEDEFAULT,
                  CW_USEDEFAULT,
                  0,
                  0,
                  Instance,
                  0);

    if (hWnd == 0) {
      error = str8_lit("failed to create graphics window");
    }  else {
      graphics_window = hWnd;
    }
  }

  HDC dc;
  HGLRC graphics_context = 0;
  int format_idx = 0;
  if (!error) {
    dc = GetDC(graphics_window);

    int format_attribs[] = {
      WGL_DRAW_TO_WINDOW_ARB, TRUE,
      WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
      WGL_SWAP_METHOD_ARB, WGL_SWAP_EXCHANGE_ARB,
      WGL_SUPPORT_OPENGL_ARB, TRUE,
      WGL_DOUBLE_BUFFER_ARB, TRUE,
      WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
      WGL_COLOR_BITS_ARB, 24,
      WGL_RED_BITS_ARB,    8,
      WGL_GREEN_BITS_ARB,  8,
      WGL_BLUE_BITS_ARB,   8,
      0
    };

    UINT num_formats = 0;
    BOOL cpf = win32_wglChoosePixelFormatARB(dc, format_attribs, 0,
                                             1, &format_idx, &num_formats);

    if (!cpf || num_formats == 0) {
      error = str8_lit("failed to choose graphics pixel format");
      ReleaseDC(graphics_window, dc);
    }
  }

  if (!error) {
    PIXELFORMATDESCRIPTOR format_desc = {};
    BOOL spf = SetPixelFormat(dc, format_idx, &format_desc);
    if (!spf) {
      error = str8_lit("failed to set graphics pixel format");
      ReleaseDC(graphics_window, dc);
    }
  }

  if (!error) {
    int attribs[] = {
      WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
      WGL_CONTEXT_MINOR_VERSION_ARB, 6,
      WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
      WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
      0
    };

    graphics_context = win32_wglCreateContextAttribsARB(dc, 0, attribs);
    if (graphics_context == 0) {
      error = str8_lit("failed to create graphics context.");
      ReleaseDC(graphics_window, dc);
    }
  }

  if (!error)
  {
    ReleaseDC(graphics_window, dc);
  }

  if (!error) {
    error = gl_init();
  }

  if (!error) 
  {
    HDC dc = GetDC(graphics_window);
    win32_wglMakeCurrent(dc, graphics_context);

    error = gl_ext_init(win32_wglGetProcAddress);

    ReleaseDC(graphics_window, dc);
  }

done:
  W32_OpenGLWindow window = {};
  if (error.size > 0) {
    printf("Error: %.*s\n", str8_expand(error));
  } else {
    window.window = graphics_window;
    window.glrc = graphics_context;
  }
  return window;
}

function String8
win32_wgl_init(HINSTANCE Instance) {
  String8 error = {};

  // Load opengl32.dll and get WGL function pointers
  HINSTANCE opengl_module = LoadLibraryA("opengl32.dll");

  if (opengl_module == 0) {
    error = str8_lit("failed to initialize opengl32.dll");
  }

# define X(N,R,P) if (!error) {\
    GET_PROC_ADDRESS(win32_##N, opengl_module, #N);\
    if (!(win32_##N)) {\
      error = str8_lit("Failed to load " #N "\n");\
    }\
  }
# include "win32/win32_wgl_procs.inc"
# undef X

  // Create bootstrap window so we can get a GL render context
  char* BOOTSTRAP_WINDOW_CLASS_NAME = "bootstrap_window_class";
  HWND bootstrap_window;
  WNDCLASSA window_class = {};
  if (!error) {
    window_class.lpfnWndProc = bootstrap_wndproc;
    window_class.hInstance = Instance;
    window_class.lpszClassName = BOOTSTRAP_WINDOW_CLASS_NAME;
    window_class.style = CS_OWNDC;
    window_class.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);

    if (!RegisterClass(&window_class)) {
      error = str8_lit("failed to register window class");
    }
  }

  if (!error) {
    HWND hWnd = CreateWindowA(window_class.lpszClassName,
                  "My Window",
                  WS_OVERLAPPEDWINDOW,
                  CW_USEDEFAULT,
                  CW_USEDEFAULT,
                  CW_USEDEFAULT,
                  CW_USEDEFAULT,
                  0,
                  0,
                  Instance,
                  0);

    if (hWnd == 0) {
      error = str8_lit("failed to create bootstrap window");
    }  else {
      bootstrap_window = hWnd;
    }
  }

  HDC dc;
  HGLRC hglrc;
  if (!error) {
    dc = GetWindowDC(bootstrap_window);

    PIXELFORMATDESCRIPTOR pfd = {
      sizeof(PIXELFORMATDESCRIPTOR),
      1,
      PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, // Flags
      PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
      32,                   // Colordepth of the framebuffer.
      0, 0, 0, 0, 0, 0,
      0,
      0,
      0,
      0, 0, 0, 0,
      24,                  // Number of bits for the depthbuffer
      8,                  // Number of bits for the stencilbuffer
      0,                 // Number of Aux buffers in the framebuffer.
      PFD_MAIN_PLANE,
      0,
      0, 0, 0
    };

    HDC ourWindowHandleToDeviceContext = GetDC(bootstrap_window);

    int letWindowsChooseThisPixelFormat;
    letWindowsChooseThisPixelFormat = ChoosePixelFormat(ourWindowHandleToDeviceContext, &pfd);
    SetPixelFormat(ourWindowHandleToDeviceContext, letWindowsChooseThisPixelFormat, &pfd);

    hglrc = win32_wglCreateContext(ourWindowHandleToDeviceContext);
    win32_wglMakeCurrent(ourWindowHandleToDeviceContext, hglrc);

    error = win32_wgl_ext_init(win32_wglGetProcAddress);
  }

  if (!error) {
    ReleaseDC(bootstrap_window, dc);

    HGLRC bootstrap_context = hglrc;

    // Destroy bootstrap window, context and class
    {
      HDC dc = GetDC(bootstrap_window);
      win32_wglMakeCurrent(0, 0);
      ReleaseDC(bootstrap_window, dc);
      BOOL delete_context = win32_wglDeleteContext(bootstrap_context);
      Assert(delete_context);

      BOOL destroy_window = DestroyWindow(bootstrap_window);
      Assert(destroy_window);

      BOOL unregister = UnregisterClassA(BOOTSTRAP_WINDOW_CLASS_NAME, Instance);
      Assert(unregister);
    }
  }

  return error;
}

function String8
win32_wgl_ext_init(W32_wglGetProcAddressFunc v_wglGetProcAddress) {
  String8 error = {};

# define WGL_GET_PROC_ADDRESS(v,n) (*(PROC*)(&(v))) = v_wglGetProcAddress((n))

  // Load WGL extension functions
# define X(N,R,P) if (!error) {\
  WGL_GET_PROC_ADDRESS(win32_##N, #N);\
  if (win32_##N == 0) {\
    error = str8_lit("failed to initialize " #N);\
  }\
}
# include "win32/win32_wgl_ext_procs.inc"
# undef X

  return error;
}
