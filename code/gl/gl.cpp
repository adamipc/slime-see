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

function GLuint
gl_create_shader_program(M_Arena *arena, String8 vss, String8 fss, String8List feedback_varyings) {
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(vertexShader, 1, (char**)&vss.str, (GLint*)&vss.size);
  glCompileShader(vertexShader);

  int success;
  char infoLog[512];

  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
  }

  if (success) {
    glShaderSource(fragmentShader, 1, (char**)&fss.str, (GLint*)&fss.size);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
      printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
    }
  }

  GLuint shaderProgram = glCreateProgram();
  if (success) {
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    if (feedback_varyings.node_count > 0) {
      M_Scratch scratch(arena);
      M_Temp restore_point = m_begin_temp(arena);
      StringJoin join = {};
      join.post = str8_lit("\0");
      join.mid = str8_lit("\0");
      String8 feedback_varyings_array = str8_join(scratch, &feedback_varyings, &join);
      glTransformFeedbackVaryings(shaderProgram, 1, (char**)&feedback_varyings_array.str, GL_SEPARATE_ATTRIBS);
      m_end_temp(restore_point);
    }

    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
      printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    }
  }

  GLuint result = 0;
  if (success) {
    glUseProgram(shaderProgram);
    result = shaderProgram;
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return result;
}

