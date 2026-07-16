// glapi.cpp — desktop GL function loading via SDL_GL_GetProcAddress.

#ifndef __ANDROID__

#include "glapi.h"
#include <SDL.h>

void (*glActiveTexture)(GLenum);
void (*glAttachShader)(GLuint, GLuint);
void (*glBindTexture)(GLenum, GLuint);
void (*glBlendFunc)(GLenum, GLenum);
void (*glClear)(GLbitfield);
void (*glClearColor)(GLclampf, GLclampf, GLclampf, GLclampf);
void (*glClearDepthf_port)(GLclampf);
void (*glCompileShader)(GLuint);
GLuint (*glCreateProgram)(void);
GLuint (*glCreateShader)(GLenum);
void (*glCullFace)(GLenum);
void (*glDeleteProgram)(GLuint);
void (*glDeleteShader)(GLuint);
void (*glDeleteTextures)(GLsizei, const GLuint*);
void (*glDepthFunc)(GLenum);
void (*glDepthMask)(GLboolean);
void (*glDisable)(GLenum);
void (*glDisableVertexAttribArray)(GLuint);
void (*glDrawArrays)(GLenum, GLint, GLsizei);
void (*glDrawElements)(GLenum, GLsizei, GLenum, const void*);
void (*glEnable)(GLenum);
void (*glEnableVertexAttribArray)(GLuint);
void (*glFrontFace)(GLenum);
void (*glGenTextures)(GLsizei, GLuint*);
void (*glGenerateMipmap)(GLenum);
GLenum (*glGetError)(void);
void (*glGetIntegerv)(GLenum, GLint*);
void (*glGetProgramInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*);
void (*glGetProgramiv)(GLuint, GLenum, GLint*);
void (*glGetShaderInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*);
void (*glGetShaderiv)(GLuint, GLenum, GLint*);
GLint (*glGetUniformLocation)(GLuint, const GLchar*);
void (*glBindAttribLocation)(GLuint, GLuint, const GLchar*);
void (*glLinkProgram)(GLuint);
void (*glPixelStorei)(GLenum, GLint);
void (*glPolygonOffset)(GLfloat, GLfloat);
void (*glReadPixels)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void*);
void (*glScissor)(GLint, GLint, GLsizei, GLsizei);
void (*glShaderSource)(GLuint, GLsizei, const GLchar* const*, const GLint*);
void (*glTexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*);
void (*glTexParameteri)(GLenum, GLenum, GLint);
void (*glTexSubImage2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void*);
void (*glUniform1f)(GLint, GLfloat);
void (*glUniform1i)(GLint, GLint);
void (*glUniform2f)(GLint, GLfloat, GLfloat);
void (*glUniform2fv)(GLint, GLsizei, const GLfloat*);
void (*glUniform3f)(GLint, GLfloat, GLfloat, GLfloat);
void (*glUniform3fv)(GLint, GLsizei, const GLfloat*);
void (*glUniform4f)(GLint, GLfloat, GLfloat, GLfloat, GLfloat);
void (*glUniform4fv)(GLint, GLsizei, const GLfloat*);
void (*glUniformMatrix4fv)(GLint, GLsizei, GLboolean, const GLfloat*);
void (*glUseProgram)(GLuint);
void (*glVertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
void (*glViewport)(GLint, GLint, GLsizei, GLsizei);

// desktop GL exposes glClearDepth(double); wrap it to the GLES-style float call
static void (*s_glClearDepthD)(double);
static void ClearDepthfWrap(GLclampf d) { s_glClearDepthD((double)d); }

#define LOAD(name) \
    *(void**)(&name) = SDL_GL_GetProcAddress(#name); \
    if (name == NULL) { SDL_Log("GL load failed: %s", #name); ok = 0; }

int PortGLInit(void)
{
    int ok = 1;
    LOAD(glActiveTexture)
    LOAD(glAttachShader)
    LOAD(glBindTexture)
    LOAD(glBlendFunc)
    LOAD(glClear)
    LOAD(glClearColor)
    LOAD(glCompileShader)
    LOAD(glCreateProgram)
    LOAD(glCreateShader)
    LOAD(glCullFace)
    LOAD(glDeleteProgram)
    LOAD(glDeleteShader)
    LOAD(glDeleteTextures)
    LOAD(glDepthFunc)
    LOAD(glDepthMask)
    LOAD(glDisable)
    LOAD(glDisableVertexAttribArray)
    LOAD(glDrawArrays)
    LOAD(glDrawElements)
    LOAD(glEnable)
    LOAD(glEnableVertexAttribArray)
    LOAD(glFrontFace)
    LOAD(glGenTextures)
    LOAD(glGenerateMipmap)
    LOAD(glGetError)
    LOAD(glGetIntegerv)
    LOAD(glGetProgramInfoLog)
    LOAD(glGetProgramiv)
    LOAD(glGetShaderInfoLog)
    LOAD(glGetShaderiv)
    LOAD(glGetUniformLocation)
    LOAD(glBindAttribLocation)
    LOAD(glLinkProgram)
    LOAD(glPixelStorei)
    LOAD(glPolygonOffset)
    LOAD(glReadPixels)
    LOAD(glScissor)
    LOAD(glShaderSource)
    LOAD(glTexImage2D)
    LOAD(glTexParameteri)
    LOAD(glTexSubImage2D)
    LOAD(glUniform1f)
    LOAD(glUniform1i)
    LOAD(glUniform2f)
    LOAD(glUniform2fv)
    LOAD(glUniform3f)
    LOAD(glUniform3fv)
    LOAD(glUniform4f)
    LOAD(glUniform4fv)
    LOAD(glUniformMatrix4fv)
    LOAD(glUseProgram)
    LOAD(glVertexAttribPointer)
    LOAD(glViewport)

    *(void**)(&s_glClearDepthD) = SDL_GL_GetProcAddress("glClearDepth");
    if (s_glClearDepthD == NULL)
    {
        *(void**)(&glClearDepthf_port) = SDL_GL_GetProcAddress("glClearDepthf");
        if (glClearDepthf_port == NULL) { SDL_Log("GL load failed: glClearDepth"); ok = 0; }
    }
    else
    {
        glClearDepthf_port = ClearDepthfWrap;
    }
    return ok;
}

#endif // !__ANDROID__
