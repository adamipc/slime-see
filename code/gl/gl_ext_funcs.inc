X(glGenBuffers, void, (GLsizei n, GLuint *buffers))
X(glBindBuffer, void, (GLenum target, GLuint buffer))
X(glBufferData, void, (GLenum target, GLsizeiptr size, const void *data, GLenum usage))
X(glCreateShader, GLuint, (GLenum type))
X(glShaderSource, void, (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length))
X(glCompileShader, void, (GLuint shader))
X(glCreateProgram, GLuint, (void))
X(glAttachShader, void, (GLuint program, GLuint shader))
X(glLinkProgram, void, (GLuint program))

X(glGetShaderInfoLog, void, (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog))
X(glGetProgramInfoLog, void, (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog))
X(glGetShaderiv, void, (GLuint shader, GLenum pname, GLint *params))
X(glGetProgramiv, void, (GLuint program, GLenum pname, GLint *params))
X(glUseProgram, void, (GLuint program))
X(glDeleteShader, void, (GLuint shader))
X(glVertexAttribPointer, void, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer))
X(glEnableVertexAttribArray, void, (GLuint index))
X(glGenVertexArrays, void, (GLsizei n, GLuint *arrays))
X(glBindVertexArray, void, (GLuint array))
X(glGenFramebuffers, void, (GLsizei n, GLuint *framebuffers))
X(glUniform4f, void, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3))
X(glGetUniformLocation, GLint, (GLuint program, const GLchar *name))
X(glTransformFeedbackVaryings, void, (GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode))
X(glGetAttribLocation, GLint, (GLuint program, const GLchar *name))
X(glBindTransformFeedback, void, (GLenum target, GLuint id))
X(glGenTransformFeedbacks, void, (GLsizei n, GLuint *ids))
X(glUniform1i, void, (GLint location, GLint v0))
X(glBindFramebuffer, void, (GLenum target, GLuint framebuffer))
X(glFramebufferTexture2D, void, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level))
X(glActiveTexture, void, (GLenum texture))
X(glUniform1f, void, (GLint location, GLfloat v0))
X(glDebugMessageCallback, void, (GLDEBUGPROC callback, const void *userParam))
X(glCheckFramebufferStatus, GLenum, (GLenum target))
X(glGenRenderbuffers, void, (GLsizei n, GLuint *renderbuffers))
X(glBindRenderbuffer, void, (GLenum target, GLuint renderbuffer))
X(glRenderbufferStorage, void, (GLenum target, GLenum internalformat, GLsizei width, GLsizei height))
X(glFramebufferRenderbuffer, void, (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer))
X(glGetFramebufferAttachmentParameteriv, void, (GLenum target, GLenum attachment, GLenum pname, GLint *params))
X(glBindBufferBase, void, (GLenum target, GLuint index, GLuint buffer))
X(glBeginTransformFeedback, void, (GLenum primitiveMode))
X(glEndTransformFeedback, void, (void))
X(glDeleteBuffers, void, (GLsizei n, const GLuint *buffers))
X(glClearTexImage, void, (GLuint texture, GLint level, GLenum format, GLenum type, const void *data))

