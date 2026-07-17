/*
 * This file is part of the BlupiMania 2 Android port.
 * Copyright (C) 2001, Daniel Roux & EPSITEC SA (original game)
 * Copyright (C) 2026, Vilmos Cseke (Android port)
 * https://blupi.org; https://github.com/sid14925/blupimania2-android
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://gnu.org/licenses
 */

// glapi.h — OpenGL ES 2.0 entry points for the port.
// Android: direct GLES2 headers. Desktop: pointers loaded via SDL.

#ifndef _PORT_GLAPI_H_
#define _PORT_GLAPI_H_

#ifdef __ANDROID__

#include <GLES2/gl2.h>
static inline int PortGLInit(void) { return 1; }

#else // desktop: load a GLES2-compatible subset of desktop GL

#include <stddef.h>

typedef unsigned int   GLenum;
typedef unsigned char  GLboolean;
typedef unsigned int   GLbitfield;
typedef signed char    GLbyte;
typedef short          GLshort;
typedef int            GLint;
typedef int             GLsizei;
typedef unsigned char  GLubyte;
typedef unsigned short GLushort;
typedef unsigned int   GLuint;
typedef float          GLfloat;
typedef float          GLclampf;
typedef char           GLchar;
typedef ptrdiff_t      GLsizeiptr;
typedef ptrdiff_t      GLintptr;

#define GL_FALSE                    0
#define GL_TRUE                     1
#define GL_NO_ERROR                 0
#define GL_ZERO                     0
#define GL_ONE                      1
#define GL_TRIANGLES                0x0004
#define GL_TRIANGLE_STRIP           0x0005
#define GL_TRIANGLE_FAN             0x0006
#define GL_SRC_COLOR                0x0300
#define GL_ONE_MINUS_SRC_COLOR      0x0301
#define GL_SRC_ALPHA                0x0302
#define GL_ONE_MINUS_SRC_ALPHA      0x0303
#define GL_DST_ALPHA                0x0304
#define GL_ONE_MINUS_DST_ALPHA      0x0305
#define GL_DST_COLOR                0x0306
#define GL_ONE_MINUS_DST_COLOR      0x0307
#define GL_SRC_ALPHA_SATURATE       0x0308
#define GL_FRONT                    0x0404
#define GL_BACK                     0x0405
#define GL_FRONT_AND_BACK           0x0408
#define GL_CULL_FACE                0x0B44
#define GL_BLEND                    0x0BE2
#define GL_DITHER                   0x0BD0
#define GL_SCISSOR_TEST             0x0C11
#define GL_DEPTH_TEST               0x0B71
#define GL_POLYGON_OFFSET_FILL      0x8037
#define GL_NEVER                    0x0200
#define GL_LESS                     0x0201
#define GL_EQUAL                    0x0202
#define GL_LEQUAL                   0x0203
#define GL_GREATER                  0x0204
#define GL_NOTEQUAL                 0x0205
#define GL_GEQUAL                   0x0206
#define GL_ALWAYS                   0x0207
#define GL_CW                       0x0900
#define GL_CCW                      0x0901
#define GL_TEXTURE_2D               0x0DE1
#define GL_UNSIGNED_BYTE            0x1401
#define GL_UNSIGNED_SHORT           0x1403
#define GL_FLOAT                    0x1406
#define GL_RGBA                     0x1908
#define GL_RGB                      0x1907
#define GL_NEAREST                  0x2600
#define GL_LINEAR                   0x2601
#define GL_NEAREST_MIPMAP_NEAREST   0x2700
#define GL_LINEAR_MIPMAP_NEAREST    0x2701
#define GL_NEAREST_MIPMAP_LINEAR    0x2702
#define GL_LINEAR_MIPMAP_LINEAR     0x2703
#define GL_TEXTURE_MAG_FILTER       0x2800
#define GL_TEXTURE_MIN_FILTER       0x2801
#define GL_TEXTURE_WRAP_S           0x2802
#define GL_TEXTURE_WRAP_T           0x2803
#define GL_REPEAT                   0x2901
#define GL_CLAMP_TO_EDGE            0x812F
#define GL_MIRRORED_REPEAT          0x8370
#define GL_TEXTURE0                 0x84C0
#define GL_TEXTURE1                 0x84C1
#define GL_DEPTH_BUFFER_BIT         0x00000100
#define GL_COLOR_BUFFER_BIT         0x00004000
#define GL_FRAGMENT_SHADER          0x8B30
#define GL_VERTEX_SHADER            0x8B31
#define GL_COMPILE_STATUS           0x8B81
#define GL_LINK_STATUS              0x8B82
#define GL_INFO_LOG_LENGTH          0x8B84
#define GL_UNPACK_ALIGNMENT         0x0CF5
#define GL_PACK_ALIGNMENT           0x0D05
#define GL_MAX_TEXTURE_SIZE         0x0D33

extern void (*glActiveTexture)(GLenum);
extern void (*glAttachShader)(GLuint, GLuint);
extern void (*glBindTexture)(GLenum, GLuint);
extern void (*glBlendFunc)(GLenum, GLenum);
extern void (*glClear)(GLbitfield);
extern void (*glClearColor)(GLclampf, GLclampf, GLclampf, GLclampf);
extern void (*glClearDepthf_port)(GLclampf);
#define glClearDepthf glClearDepthf_port
extern void (*glCompileShader)(GLuint);
extern GLuint (*glCreateProgram)(void);
extern GLuint (*glCreateShader)(GLenum);
extern void (*glCullFace)(GLenum);
extern void (*glDeleteProgram)(GLuint);
extern void (*glDeleteShader)(GLuint);
extern void (*glDeleteTextures)(GLsizei, const GLuint*);
extern void (*glDepthFunc)(GLenum);
extern void (*glDepthMask)(GLboolean);
extern void (*glDisable)(GLenum);
extern void (*glDisableVertexAttribArray)(GLuint);
extern void (*glDrawArrays)(GLenum, GLint, GLsizei);
extern void (*glDrawElements)(GLenum, GLsizei, GLenum, const void*);
extern void (*glEnable)(GLenum);
extern void (*glEnableVertexAttribArray)(GLuint);
extern void (*glFrontFace)(GLenum);
extern void (*glGenTextures)(GLsizei, GLuint*);
extern void (*glGenerateMipmap)(GLenum);
extern GLenum (*glGetError)(void);
extern void (*glGetIntegerv)(GLenum, GLint*);
extern void (*glGetProgramInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*);
extern void (*glGetProgramiv)(GLuint, GLenum, GLint*);
extern void (*glGetShaderInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*);
extern void (*glGetShaderiv)(GLuint, GLenum, GLint*);
extern GLint (*glGetUniformLocation)(GLuint, const GLchar*);
extern void (*glBindAttribLocation)(GLuint, GLuint, const GLchar*);
extern void (*glLinkProgram)(GLuint);
extern void (*glPixelStorei)(GLenum, GLint);
extern void (*glPolygonOffset)(GLfloat, GLfloat);
extern void (*glReadPixels)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void*);
extern void (*glScissor)(GLint, GLint, GLsizei, GLsizei);
extern void (*glShaderSource)(GLuint, GLsizei, const GLchar* const*, const GLint*);
extern void (*glTexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*);
extern void (*glTexParameteri)(GLenum, GLenum, GLint);
extern void (*glTexSubImage2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void*);
extern void (*glUniform1f)(GLint, GLfloat);
extern void (*glUniform1i)(GLint, GLint);
extern void (*glUniform2f)(GLint, GLfloat, GLfloat);
extern void (*glUniform2fv)(GLint, GLsizei, const GLfloat*);
extern void (*glUniform3f)(GLint, GLfloat, GLfloat, GLfloat);
extern void (*glUniform3fv)(GLint, GLsizei, const GLfloat*);
extern void (*glUniform4f)(GLint, GLfloat, GLfloat, GLfloat, GLfloat);
extern void (*glUniform4fv)(GLint, GLsizei, const GLfloat*);
extern void (*glUniformMatrix4fv)(GLint, GLsizei, GLboolean, const GLfloat*);
extern void (*glUseProgram)(GLuint);
extern void (*glVertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
extern void (*glViewport)(GLint, GLint, GLsizei, GLsizei);

int PortGLInit(void);   // returns 1 on success; call after GL context creation

#endif // !__ANDROID__

#endif // _PORT_GLAPI_H_
