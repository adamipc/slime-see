#include "win32/win32_wgl.h"

LRESULT CALLBACK bootstrap_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message)
  {
    case WM_CLOSE:
    case WM_DESTROY: {
      PostQuitMessage(0);
    } break;
    default: {
      return DefWindowProc(hWnd, message, wParam, lParam);
    } break;
  }

  return 0;
}

function W32_OpenGLWindow
win32_create_opengl_window(HINSTANCE Instance, WNDPROC window_proc, int width, int height) {
  String8 error = {};

  W32_OpenGLWindow window = {};
  WNDCLASSA window_class = {};
  window_class.lpfnWndProc = window_proc;
  window_class.hInstance = Instance;
  window_class.lpszClassName = "graphics_window";
  window_class.hCursor = NULL;

  if (!RegisterClass(&window_class)) {
    error = str8_lit("failed to register window class");
  }

  if (!error) {
    RECT window_rect = {};
    window_rect.left = 50;
    window_rect.top = 50;
    window_rect.right = window_rect.left + width;
    window_rect.bottom = window_rect.top + height;
    AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, true);
    HWND graphics_window = CreateWindowA(window_class.lpszClassName,
        "My Window",
        WS_OVERLAPPEDWINDOW,
        window_rect.left,
        window_rect.top,
        window_rect.right - window_rect.left,
        window_rect.bottom - window_rect.top,
        0,
        0,
        Instance,
        0);

    if (graphics_window == 0) {
      error = str8_lit("failed to create graphics window");
    }

    if (!error) {
      HGLRC graphics_context = 0;
      int format_idx = 0;
      if (!error) {
        HDC dc = GetDC(graphics_window);

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
            WGL_CONTEXT_MINOR_VERSION_ARB, 4,
            WGL_CONTEXT_FLAGS_ARB, 0,
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
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
          dc = GetDC(graphics_window);
          win32_wglMakeCurrent(dc, graphics_context);

          error = gl_ext_init(win32_wglGetProcAddress);

          ReleaseDC(graphics_window, dc);
        }

        if (error.size > 0) {
          printf("Error: %.*s\n", str8_expand(error));
        } else {
          window.window = graphics_window;
          window.glrc = graphics_context;
        }
      }
    }
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
    HWND bootstrap_window = CreateWindowA(window_class.lpszClassName,
                  "My Window",
                  WS_BORDER,
                  CW_USEDEFAULT,
                  CW_USEDEFAULT,
                  CW_USEDEFAULT,
                  CW_USEDEFAULT,
                  0,
                  0,
                  Instance,
                  0);

    if (bootstrap_window == 0) {
      error = str8_lit("failed to create bootstrap window");
    }

    if (!error) {
      if (!error) {
        HDC dc = GetWindowDC(bootstrap_window);

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

        HGLRC bootstrap_context = win32_wglCreateContext(ourWindowHandleToDeviceContext);
        win32_wglMakeCurrent(ourWindowHandleToDeviceContext, bootstrap_context);

        error = win32_wgl_ext_init(win32_wglGetProcAddress);

        if (!error) {
          // Destroy bootstrap window, context and class
          win32_wglMakeCurrent(0, 0);
          ReleaseDC(bootstrap_window, dc);
          BOOL delete_context = win32_wglDeleteContext(bootstrap_context);
          Assert(delete_context);

          BOOL destroy_window = DestroyWindow(bootstrap_window);
          Assert(destroy_window);

          BOOL unregister = UnregisterClassA(BOOTSTRAP_WINDOW_CLASS_NAME, Instance);
          Assert(unregister);

          // Clear the message queue
          MSG msg = {};
          while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
          }
        }
      }
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
