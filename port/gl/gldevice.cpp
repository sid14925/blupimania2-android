// gldevice.cpp — IDirect3DDevice7 implemented over OpenGL ES 2.0.
// Emulates the D3D7 fixed-function pipeline (transforms, per-vertex lighting,
// two texture stages, linear vertex fog, alpha test) with a single übershader
// driven by uniforms.

#include <windows.h>
#include <d3d.h>
#include "glapi.h"

#include <SDL.h>
#include <math.h>

#define MAX_LIGHTS      8       // engine-side light slots
#define MAX_GPU_LIGHTS  4       // lights actually sent to the shader

// ---------------------------------------------------------------------------
// IDirectDrawSurface7 — CPU ARGB pixel buffer + GL texture
// ---------------------------------------------------------------------------

static BOOL IsPow2(int v) { return v > 0 && (v & (v-1)) == 0; }

IDirectDrawSurface7::IDirectDrawSurface7(int width, int height, BOOL bAlpha)
{
    m_width     = width;
    m_height    = height;
    m_bAlpha    = bAlpha;
    m_bDirty    = TRUE;
    m_pixels    = (DWORD*)calloc((size_t)width*height, sizeof(DWORD));
    m_glTexture = 0;
    m_refCount  = 1;
}

IDirectDrawSurface7::~IDirectDrawSurface7()
{
    InvalidateGL();
    free(m_pixels);
}

void IDirectDrawSurface7::FillDesc(DDSURFACEDESC2* d)
{
    memset(d, 0, sizeof(DDSURFACEDESC2));
    d->dwSize   = sizeof(DDSURFACEDESC2);
    d->dwFlags  = DDSD_WIDTH|DDSD_HEIGHT|DDSD_PITCH|DDSD_PIXELFORMAT;
    d->dwWidth  = (DWORD)m_width;
    d->dwHeight = (DWORD)m_height;
    d->lPitch   = m_width * 4;
    d->lpSurface = m_pixels;
    d->ddpfPixelFormat.dwSize        = sizeof(DDPIXELFORMAT);
    d->ddpfPixelFormat.dwFlags       = DDPF_RGB | (m_bAlpha ? DDPF_ALPHAPIXELS : 0);
    d->ddpfPixelFormat.dwRGBBitCount = 32;
    d->ddpfPixelFormat.dwRBitMask    = 0x00ff0000;
    d->ddpfPixelFormat.dwGBitMask    = 0x0000ff00;
    d->ddpfPixelFormat.dwBBitMask    = 0x000000ff;
    d->ddpfPixelFormat.dwRGBAlphaBitMask = 0xff000000;
}

HRESULT IDirectDrawSurface7::Lock(LPRECT lpDestRect, LPDDSURFACEDESC2 lpDDSurfaceDesc,
                                  DWORD dwFlags, HANDLE hEvent)
{
    (void)lpDestRect; (void)dwFlags; (void)hEvent;
    if (lpDDSurfaceDesc == NULL) return E_INVALIDARG;
    FillDesc(lpDDSurfaceDesc);
    return S_OK;
}

HRESULT IDirectDrawSurface7::Unlock(LPRECT lpRect)
{
    (void)lpRect;
    m_bDirty = TRUE;
    return S_OK;
}

HRESULT IDirectDrawSurface7::GetSurfaceDesc(LPDDSURFACEDESC2 lpDDSurfaceDesc)
{
    if (lpDDSurfaceDesc == NULL) return E_INVALIDARG;
    FillDesc(lpDDSurfaceDesc);
    return S_OK;
}

ULONG IDirectDrawSurface7::AddRef()
{
    return (ULONG)(++m_refCount);
}

ULONG IDirectDrawSurface7::Release()
{
    int rc = --m_refCount;
    if (rc <= 0) { delete this; return 0; }
    return (ULONG)rc;
}

void IDirectDrawSurface7::InvalidateGL()
{
    if (m_glTexture != 0)
    {
        glDeleteTextures(1, &m_glTexture);
        m_glTexture = 0;
    }
    m_bDirty = TRUE;
}

static int NextPow2(int v)
{
    int p = 1;
    while (p < v && p < 4096) p <<= 1;
    return p;
}

unsigned int IDirectDrawSurface7::BindGL()
{
    if (m_glTexture == 0)
    {
        glGenTextures(1, &m_glTexture);
        m_bDirty = TRUE;
    }
    glBindTexture(GL_TEXTURE_2D, m_glTexture);

    if (m_bDirty)
    {
        // convert ARGB (DWORD) -> RGBA bytes for GL
        int n = m_width * m_height;
        unsigned char* rgba = (unsigned char*)malloc((size_t)n*4);
        for (int i = 0; i < n; i++)
        {
            DWORD p = m_pixels[i];
            rgba[i*4+0] = (unsigned char)((p >> 16) & 0xff);
            rgba[i*4+1] = (unsigned char)((p >> 8) & 0xff);
            rgba[i*4+2] = (unsigned char)(p & 0xff);
            rgba[i*4+3] = (unsigned char)((p >> 24) & 0xff);
        }

        // GLES2 cannot sample NPOT textures with REPEAT wrap or mipmaps
        // ("incomplete texture" = solid black — this is what killed the
        // water on real devices). Resample NPOT images to power-of-two.
        int upW = m_width, upH = m_height;
        unsigned char* upload = rgba;
        if (!IsPow2(m_width) || !IsPow2(m_height))
        {
            upW = NextPow2(m_width);
            upH = NextPow2(m_height);
            unsigned char* scaled = (unsigned char*)malloc((size_t)upW*upH*4);
            for (int y = 0; y < upH; y++)
            {
                // bilinear resample
                float fy = (float)y * (float)(m_height-1) / (float)(upH-1 > 0 ? upH-1 : 1);
                int   y0 = (int)fy;
                int   y1 = (y0+1 < m_height) ? y0+1 : y0;
                float wy = fy - y0;
                for (int x = 0; x < upW; x++)
                {
                    float fx = (float)x * (float)(m_width-1) / (float)(upW-1 > 0 ? upW-1 : 1);
                    int   x0 = (int)fx;
                    int   x1 = (x0+1 < m_width) ? x0+1 : x0;
                    float wx = fx - x0;
                    unsigned char* d = scaled + ((size_t)y*upW + x)*4;
                    const unsigned char* s00 = rgba + ((size_t)y0*m_width + x0)*4;
                    const unsigned char* s10 = rgba + ((size_t)y0*m_width + x1)*4;
                    const unsigned char* s01 = rgba + ((size_t)y1*m_width + x0)*4;
                    const unsigned char* s11 = rgba + ((size_t)y1*m_width + x1)*4;
                    for (int c = 0; c < 4; c++)
                    {
                        float top = s00[c] + (s10[c]-s00[c])*wx;
                        float bot = s01[c] + (s11[c]-s01[c])*wx;
                        d[c] = (unsigned char)(top + (bot-top)*wy + 0.5f);
                    }
                }
            }
            upload = scaled;
            SDL_Log("texture resampled NPOT %dx%d -> %dx%d", m_width, m_height, upW, upH);
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, upW, upH, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, upload);
        glGenerateMipmap(GL_TEXTURE_2D);
        if (upload != rgba) free(upload);
        free(rgba);
        m_bDirty = FALSE;
    }
    return m_glTexture;
}

// ---------------------------------------------------------------------------
// matrix helpers (D3D row-vector convention)
// ---------------------------------------------------------------------------

static void MatIdentity(D3DMATRIX& m)
{
    memset(&m, 0, sizeof(m));
    m._11 = m._22 = m._33 = m._44 = 1.0f;
}

static void MatMul(D3DMATRIX& out, const D3DMATRIX& a, const D3DMATRIX& b)
{
    D3DMATRIX r;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            r.m[i][j] = a.m[i][0]*b.m[0][j] + a.m[i][1]*b.m[1][j] +
                        a.m[i][2]*b.m[2][j] + a.m[i][3]*b.m[3][j];
    out = r;
}

// ---------------------------------------------------------------------------
// shader
// ---------------------------------------------------------------------------

static const char* VS_SRC =
"uniform mat4 u_mvp;\n"
"uniform mat4 u_world;\n"
"uniform int  u_lighting;\n"
"uniform int  u_useVtxColor;\n"
"uniform vec4 u_matDiffuse;\n"
"uniform vec4 u_matAmbient;\n"
"uniform vec4 u_matEmissive;\n"
"uniform vec4 u_globalAmbient;\n"
"uniform int  u_lightCount;\n"
"uniform vec4 u_lightPos[4];\n"      // w=0 directional (xyz = -direction), w=1 positional
"uniform vec4 u_lightDir[4];\n"      // xyz = spot direction, w: 0=no spot, else cos(phi/2)
"uniform vec4 u_lightDiffuse[4];\n"
"uniform vec4 u_lightAmbient[4];\n"
"uniform vec4 u_lightAtten[4];\n"    // a0, a1, a2, range
"uniform vec2 u_lightSpot[4];\n"     // cos(theta/2), falloff-unused
"uniform lowp int u_fogEnable;\n"    // shared with the fragment shader: precision must match on GLES
"uniform vec2 u_fogRange;\n"         // start, end (view-space z)
"uniform mat4 u_view;\n"
"attribute vec3 a_pos;\n"
"attribute vec3 a_normal;\n"
"attribute vec4 a_color;\n"
"attribute vec2 a_uv0;\n"
"attribute vec2 a_uv1;\n"
"varying vec4 v_color;\n"
"varying vec2 v_uv0;\n"
"varying vec2 v_uv1;\n"
"varying float v_fog;\n"
"void main()\n"
"{\n"
"    vec4 pos = vec4(a_pos, 1.0);\n"
"    vec4 clip = u_mvp * pos;\n"
"    clip.z = clip.z * 2.0 - clip.w;\n"   // D3D [0,1] -> GL [-1,1] depth
"    gl_Position = clip;\n"
"\n"
"    vec4 col;\n"
"    if (u_lighting == 1)\n"
"    {\n"
"        vec3 wpos = (u_world * pos).xyz;\n"
"        vec3 wnrm = normalize((u_world * vec4(a_normal, 0.0)).xyz);\n"
"        vec3 rgb = u_matEmissive.rgb + u_matAmbient.rgb * u_globalAmbient.rgb;\n"
"        for (int i = 0; i < 4; i++)\n"
"        {\n"
"            if (i >= u_lightCount) break;\n"
"            vec3 L; float att = 1.0;\n"
"            if (u_lightPos[i].w == 0.0)\n"
"            {\n"
"                L = normalize(u_lightPos[i].xyz);\n"
"            }\n"
"            else\n"
"            {\n"
"                vec3 d = u_lightPos[i].xyz - wpos;\n"
"                float dist = length(d);\n"
"                L = d / max(dist, 0.0001);\n"
"                if (dist > u_lightAtten[i].w) att = 0.0;\n"
"                else att = 1.0 / max(u_lightAtten[i].x + u_lightAtten[i].y*dist + u_lightAtten[i].z*dist*dist, 0.0001);\n"
"                if (u_lightDir[i].w > 0.0)\n"
"                {\n"
"                    float c = dot(-L, normalize(u_lightDir[i].xyz));\n"
"                    float inner = u_lightSpot[i].x;\n"
"                    float outer = u_lightDir[i].w;\n"
"                    att *= clamp((c - outer) / max(inner - outer, 0.001), 0.0, 1.0);\n"
"                }\n"
"            }\n"
"            float ndl = max(dot(wnrm, L), 0.0);\n"
"            rgb += u_matAmbient.rgb * u_lightAmbient[i].rgb * att;\n"
"            rgb += u_matDiffuse.rgb * u_lightDiffuse[i].rgb * ndl * att;\n"
"        }\n"
"        col = vec4(clamp(rgb, 0.0, 1.0), u_matDiffuse.a);\n"
"    }\n"
"    else if (u_useVtxColor == 1)\n"
"    {\n"
"        col = a_color.zyxw;\n"   // vertex bytes are B,G,R,A
"    }\n"
"    else\n"
"    {\n"
"        col = vec4(1.0, 1.0, 1.0, 1.0);\n"
"    }\n"
"    v_color = col;\n"
"    v_uv0 = a_uv0;\n"
"    v_uv1 = a_uv1;\n"
"\n"
"    if (u_fogEnable == 1)\n"
"    {\n"
"        float z = (u_view * (u_world * pos)).z;\n"
"        v_fog = clamp((u_fogRange.y - z) / max(u_fogRange.y - u_fogRange.x, 0.001), 0.0, 1.0);\n"
"    }\n"
"    else v_fog = 1.0;\n"
"}\n";

static const char* FS_SRC =
"#ifdef GL_ES\n"
"#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
"precision highp float;\n"
"#else\n"
"precision mediump float;\n"
"#endif\n"
"#endif\n"
"uniform sampler2D u_tex0;\n"
"uniform sampler2D u_tex1;\n"
"uniform int  u_s0op;\n"
"uniform int  u_s0arg1;\n"
"uniform int  u_s0arg2;\n"
"uniform int  u_s0aop;\n"
"uniform int  u_s0aarg1;\n"
"uniform int  u_s1op;\n"
"uniform int  u_s1arg1;\n"
"uniform int  u_s1arg2;\n"
"uniform int  u_s1uv;\n"
"uniform vec4 u_tfactor;\n"
"uniform int  u_alphaTest;\n"       // 0 off; else D3DCMP value
"uniform float u_alphaRef;\n"
"uniform lowp int u_fogEnable;\n"
"uniform vec4 u_fogColor;\n"
"varying vec4 v_color;\n"
"varying vec2 v_uv0;\n"
"varying vec2 v_uv1;\n"
"varying float v_fog;\n"
"\n"
"vec4 pickArg(int a, vec4 tex, vec4 cur)\n"
"{\n"
"    if (a == 0) return v_color;\n"    // D3DTA_DIFFUSE
"    if (a == 1) return cur;\n"        // D3DTA_CURRENT
"    if (a == 2) return tex;\n"        // D3DTA_TEXTURE
"    return u_tfactor;\n"              // D3DTA_TFACTOR
"}\n"
"\n"
"vec3 combine(int op, vec3 a, vec3 b, vec3 cur)\n"
"{\n"
"    if (op == 2) return a;\n"             // SELECTARG1
"    if (op == 3) return b;\n"             // SELECTARG2
"    if (op == 4) return a * b;\n"         // MODULATE
"    if (op == 5) return a * b * 2.0;\n"   // MODULATE2X
"    if (op == 7) return a + b;\n"         // ADD
"    return cur;\n"
"}\n"
"\n"
"void main()\n"
"{\n"
"    vec4 cur = v_color;\n"
"    if (u_s0op != 1)\n"
"    {\n"
"        vec4 tex = texture2D(u_tex0, v_uv0);\n"
"        vec4 a = pickArg(u_s0arg1, tex, cur);\n"
"        vec4 b = pickArg(u_s0arg2, tex, cur);\n"
"        vec3 rgb = combine(u_s0op, a.rgb, b.rgb, cur.rgb);\n"
"        float alpha = cur.a;\n"
"        if (u_s0aop != 1)\n"
"        {\n"
"            vec4 aa = pickArg(u_s0aarg1, tex, cur);\n"
"            if (u_s0aop == 4) alpha = aa.a * v_color.a;\n"     // MODULATE
"            else alpha = aa.a;\n"                              // SELECTARG1
"        }\n"
"        cur = vec4(rgb, alpha);\n"
"    }\n"
"    if (u_s1op != 1)\n"
"    {\n"
"        vec2 uv = (u_s1uv == 1) ? v_uv1 : v_uv0;\n"
"        vec4 tex = texture2D(u_tex1, uv);\n"
"        vec4 a = pickArg(u_s1arg1, tex, cur);\n"
"        vec4 b = pickArg(u_s1arg2, tex, cur);\n"
"        vec3 rgb = combine(u_s1op, a.rgb, b.rgb, cur.rgb);\n"
"        cur = vec4(rgb, cur.a);\n"
"    }\n"
"\n"
"    if (u_alphaTest != 0)\n"
"    {\n"
"        bool pass = true;\n"
"        if      (u_alphaTest == 5) pass = (cur.a >  u_alphaRef);\n"  // GREATER
"        else if (u_alphaTest == 7) pass = (cur.a >= u_alphaRef);\n"  // GREATEREQUAL
"        else if (u_alphaTest == 2) pass = (cur.a <  u_alphaRef);\n"  // LESS
"        else if (u_alphaTest == 4) pass = (cur.a <= u_alphaRef);\n"  // LESSEQUAL
"        else if (u_alphaTest == 6) pass = (cur.a != u_alphaRef);\n"  // NOTEQUAL
"        if (!pass) discard;\n"
"    }\n"
"\n"
"    if (u_fogEnable == 1)\n"
"        cur.rgb = mix(u_fogColor.rgb, cur.rgb, v_fog);\n"
"    gl_FragColor = cur;\n"
"}\n";

// attribute locations (bound before link)
enum { ATTR_POS = 0, ATTR_NORMAL = 1, ATTR_COLOR = 2, ATTR_UV0 = 3, ATTR_UV1 = 4 };

// ---------------------------------------------------------------------------
// device implementation state
// ---------------------------------------------------------------------------

struct TexStage
{
    IDirectDrawSurface7* texture;
    DWORD colorOp, colorArg1, colorArg2;
    DWORD alphaOp, alphaArg1;
    DWORD address;          // D3DTADDRESS_*
    DWORD magFilter, minFilter;
    DWORD texCoordIndex;
};

struct GLLight
{
    D3DLIGHT7 light;
    BOOL      bEnabled;
    BOOL      bSet;
};

struct IDirect3DDevice7::Impl
{
    int width, height;

    // transforms
    D3DMATRIX world, view, proj;
    BOOL matricesDirty;
    D3DMATRIX mvp;

    // render states
    BOOL   zEnable, zWrite;
    DWORD  zFunc;
    BOOL   alphaBlend;
    DWORD  srcBlend, dstBlend;
    BOOL   alphaTest;
    DWORD  alphaFunc;
    DWORD  alphaRef;        // 0..255
    BOOL   fogEnable;
    DWORD  fogColor;
    float  fogStart, fogEnd;
    BOOL   lighting;
    DWORD  ambient;
    DWORD  tFactor;
    DWORD  cullMode;
    int    zBias;

    TexStage stage[2];
    GLLight  lights[MAX_LIGHTS];
    D3DMATERIAL7 material;
    D3DVIEWPORT7 viewport;

    // GL objects
    GLuint program;
    // uniform locations
    GLint uMvp, uWorld, uView, uLighting, uUseVtxColor;
    GLint uMatDiffuse, uMatAmbient, uMatEmissive, uGlobalAmbient;
    GLint uLightCount, uLightPos, uLightDir, uLightDiffuse, uLightAmbient, uLightAtten, uLightSpot;
    GLint uFogEnableV, uFogRange;
    GLint uTex0, uTex1;
    GLint uS0op, uS0arg1, uS0arg2, uS0aop, uS0aarg1;
    GLint uS1op, uS1arg1, uS1arg2, uS1uv;
    GLint uTfactor, uAlphaTest, uAlphaRef, uFogEnableF, uFogColor;
};

static GLuint CompileShader(GLenum type, const char* src)
{
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, NULL);
    glCompileShader(sh);
    GLint ok = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[2048];
        glGetShaderInfoLog(sh, sizeof(log), NULL, log);
        SDL_Log("Shader compile error: %s", log);
        return 0;
    }
    return sh;
}

IDirect3DDevice7::IDirect3DDevice7()
{
    m = new Impl();
    memset(m, 0, sizeof(Impl));
    MatIdentity(m->world);
    MatIdentity(m->view);
    MatIdentity(m->proj);
    m->matricesDirty = TRUE;

    m->zEnable = TRUE;
    m->zWrite  = TRUE;
    m->zFunc   = D3DCMP_LESSEQUAL;
    m->srcBlend = D3DBLEND_ONE;
    m->dstBlend = D3DBLEND_ZERO;
    m->alphaFunc = D3DCMP_ALWAYS;
    m->cullMode = D3DCULL_CCW;
    m->fogStart = 0.0f;
    m->fogEnd   = 1.0f;
    m->lighting = TRUE;
    m->ambient  = 0xff202020;

    for (int i = 0; i < 2; i++)
    {
        m->stage[i].colorOp   = (i == 0) ? D3DTOP_MODULATE : D3DTOP_DISABLE;
        m->stage[i].colorArg1 = D3DTA_TEXTURE;
        m->stage[i].colorArg2 = D3DTA_CURRENT;
        m->stage[i].alphaOp   = (i == 0) ? D3DTOP_SELECTARG1 : D3DTOP_DISABLE;
        m->stage[i].alphaArg1 = D3DTA_TEXTURE;
        m->stage[i].address   = D3DTADDRESS_WRAP;
        m->stage[i].magFilter = D3DTFG_LINEAR;
        m->stage[i].minFilter = D3DTFN_LINEAR;
        m->stage[i].texCoordIndex = (DWORD)i;
    }

    m->material.diffuse.r = m->material.diffuse.g = m->material.diffuse.b = 1.0f;
    m->material.diffuse.a = 1.0f;

    m->viewport.dwX = 0; m->viewport.dwY = 0;
    m->viewport.dwWidth = 640; m->viewport.dwHeight = 480;
    m->viewport.dvMinZ = 0.0f; m->viewport.dvMaxZ = 1.0f;
}

IDirect3DDevice7::~IDirect3DDevice7()
{
    if (m->program != 0) glDeleteProgram(m->program);
    delete m;
}

HRESULT IDirect3DDevice7::InitDevice(int width, int height)
{
    m->width = width;
    m->height = height;
    m->viewport.dwWidth  = (DWORD)width;
    m->viewport.dwHeight = (DWORD)height;

    GLuint vs = CompileShader(GL_VERTEX_SHADER, VS_SRC);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, FS_SRC);
    if (vs == 0 || fs == 0) return E_FAIL;

    m->program = glCreateProgram();
    glAttachShader(m->program, vs);
    glAttachShader(m->program, fs);
    glBindAttribLocation(m->program, ATTR_POS,    "a_pos");
    glBindAttribLocation(m->program, ATTR_NORMAL, "a_normal");
    glBindAttribLocation(m->program, ATTR_COLOR,  "a_color");
    glBindAttribLocation(m->program, ATTR_UV0,    "a_uv0");
    glBindAttribLocation(m->program, ATTR_UV1,    "a_uv1");
    glLinkProgram(m->program);
    GLint ok = 0;
    glGetProgramiv(m->program, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[2048];
        glGetProgramInfoLog(m->program, sizeof(log), NULL, log);
        SDL_Log("Program link error: %s", log);
        return E_FAIL;
    }
    glDeleteShader(vs);
    glDeleteShader(fs);

    glUseProgram(m->program);
    #define U(name) glGetUniformLocation(m->program, name)
    m->uMvp = U("u_mvp"); m->uWorld = U("u_world"); m->uView = U("u_view");
    m->uLighting = U("u_lighting"); m->uUseVtxColor = U("u_useVtxColor");
    m->uMatDiffuse = U("u_matDiffuse"); m->uMatAmbient = U("u_matAmbient");
    m->uMatEmissive = U("u_matEmissive"); m->uGlobalAmbient = U("u_globalAmbient");
    m->uLightCount = U("u_lightCount"); m->uLightPos = U("u_lightPos[0]");
    m->uLightDir = U("u_lightDir[0]"); m->uLightDiffuse = U("u_lightDiffuse[0]");
    m->uLightAmbient = U("u_lightAmbient[0]"); m->uLightAtten = U("u_lightAtten[0]");
    m->uLightSpot = U("u_lightSpot[0]");
    m->uFogEnableV = U("u_fogEnable"); m->uFogRange = U("u_fogRange");
    m->uTex0 = U("u_tex0"); m->uTex1 = U("u_tex1");
    m->uS0op = U("u_s0op"); m->uS0arg1 = U("u_s0arg1"); m->uS0arg2 = U("u_s0arg2");
    m->uS0aop = U("u_s0aop"); m->uS0aarg1 = U("u_s0aarg1");
    m->uS1op = U("u_s1op"); m->uS1arg1 = U("u_s1arg1"); m->uS1arg2 = U("u_s1arg2");
    m->uS1uv = U("u_s1uv");
    m->uTfactor = U("u_tfactor"); m->uAlphaTest = U("u_alphaTest");
    m->uAlphaRef = U("u_alphaRef");
    m->uFogEnableF = m->uFogEnableV;
    m->uFogColor = U("u_fogColor");
    #undef U

    glUniform1i(m->uTex0, 0);
    glUniform1i(m->uTex1, 1);

    glViewport(0, 0, width, height);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CW);     // D3D front faces are clockwise
    return S_OK;
}

void IDirect3DDevice7::Resize(int width, int height)
{
    m->width = width;
    m->height = height;
    glViewport(0, 0, width, height);
}

void IDirect3DDevice7::Present()
{
    // buffer swap handled by the SDL layer
}

// ------------------------------------------------------------------- basics

HRESULT IDirect3DDevice7::BeginScene() { return S_OK; }
HRESULT IDirect3DDevice7::EndScene()   { return S_OK; }

static void ColorToFloats(DWORD c, float* out)
{
    out[0] = ((c >> 16) & 0xff) / 255.0f;
    out[1] = ((c >> 8) & 0xff) / 255.0f;
    out[2] = (c & 0xff) / 255.0f;
    out[3] = ((c >> 24) & 0xff) / 255.0f;
}

HRESULT IDirect3DDevice7::Clear(DWORD dwCount, LPD3DRECT lpRects, DWORD dwFlags,
                                D3DCOLOR dwColor, D3DVALUE dvZ, DWORD dwStencil)
{
    (void)dwCount; (void)lpRects; (void)dwStencil;
    GLbitfield mask = 0;
    if (dwFlags & D3DCLEAR_TARGET)
    {
        float c[4];
        ColorToFloats(dwColor, c);
        glClearColor(c[0], c[1], c[2], c[3]);
        mask |= GL_COLOR_BUFFER_BIT;
    }
    if (dwFlags & D3DCLEAR_ZBUFFER)
    {
        glClearDepthf(dvZ);
        glDepthMask(GL_TRUE);
        mask |= GL_DEPTH_BUFFER_BIT;
    }
    glClear(mask);
    if ((dwFlags & D3DCLEAR_ZBUFFER) && !m->zWrite)
        glDepthMask(GL_FALSE);
    return S_OK;
}

// --------------------------------------------------------------- transforms

HRESULT IDirect3DDevice7::SetTransform(D3DTRANSFORMSTATETYPE t, LPD3DMATRIX mat)
{
    if (mat == NULL) return E_INVALIDARG;
    switch (t)
    {
        case D3DTRANSFORMSTATE_WORLD:      m->world = *mat; break;
        case D3DTRANSFORMSTATE_VIEW:       m->view  = *mat; break;
        case D3DTRANSFORMSTATE_PROJECTION: m->proj  = *mat; break;
        default: return S_OK;
    }
    m->matricesDirty = TRUE;
    return S_OK;
}

HRESULT IDirect3DDevice7::GetTransform(D3DTRANSFORMSTATETYPE t, LPD3DMATRIX mat)
{
    if (mat == NULL) return E_INVALIDARG;
    switch (t)
    {
        case D3DTRANSFORMSTATE_WORLD:      *mat = m->world; break;
        case D3DTRANSFORMSTATE_VIEW:       *mat = m->view;  break;
        case D3DTRANSFORMSTATE_PROJECTION: *mat = m->proj;  break;
        default: MatIdentity(*mat); break;
    }
    return S_OK;
}

void IDirect3DDevice7::UpdateMatrices()
{
    if (!m->matricesDirty) return;
    D3DMATRIX wv;
    MatMul(wv, m->world, m->view);
    MatMul(m->mvp, wv, m->proj);
    m->matricesDirty = FALSE;
}

HRESULT IDirect3DDevice7::SetViewport(LPD3DVIEWPORT7 vp)
{
    if (vp == NULL) return E_INVALIDARG;
    m->viewport = *vp;
    // D3D viewport origin is top-left, GL bottom-left
    glViewport((GLint)vp->dwX,
               (GLint)(m->height - (int)vp->dwY - (int)vp->dwHeight),
               (GLsizei)vp->dwWidth, (GLsizei)vp->dwHeight);
    return S_OK;
}

HRESULT IDirect3DDevice7::GetViewport(LPD3DVIEWPORT7 vp)
{
    if (vp == NULL) return E_INVALIDARG;
    *vp = m->viewport;
    return S_OK;
}

// ----------------------------------------------------------- lights/material

HRESULT IDirect3DDevice7::SetMaterial(LPD3DMATERIAL7 mat)
{
    if (mat == NULL) return E_INVALIDARG;
    m->material = *mat;
    return S_OK;
}

HRESULT IDirect3DDevice7::GetMaterial(LPD3DMATERIAL7 mat)
{
    if (mat == NULL) return E_INVALIDARG;
    *mat = m->material;
    return S_OK;
}

HRESULT IDirect3DDevice7::SetLight(DWORD idx, LPD3DLIGHT7 light)
{
    if (idx >= MAX_LIGHTS || light == NULL) return E_INVALIDARG;
    m->lights[idx].light = *light;
    m->lights[idx].bSet = TRUE;
    return S_OK;
}

HRESULT IDirect3DDevice7::GetLight(DWORD idx, LPD3DLIGHT7 light)
{
    if (idx >= MAX_LIGHTS || light == NULL) return E_INVALIDARG;
    *light = m->lights[idx].light;
    return S_OK;
}

HRESULT IDirect3DDevice7::LightEnable(DWORD idx, BOOL bEnable)
{
    if (idx >= MAX_LIGHTS) return E_INVALIDARG;
    m->lights[idx].bEnabled = bEnable;
    return S_OK;
}

HRESULT IDirect3DDevice7::GetLightEnable(DWORD idx, BOOL* pb)
{
    if (idx >= MAX_LIGHTS || pb == NULL) return E_INVALIDARG;
    *pb = m->lights[idx].bEnabled;
    return S_OK;
}

// ------------------------------------------------------------- render state

HRESULT IDirect3DDevice7::SetRenderState(D3DRENDERSTATETYPE state, DWORD value)
{
    switch (state)
    {
        case D3DRENDERSTATE_ZENABLE:          m->zEnable = (value != 0); break;
        case D3DRENDERSTATE_ZWRITEENABLE:     m->zWrite = (value != 0); break;
        case D3DRENDERSTATE_ZFUNC:            m->zFunc = value; break;
        case D3DRENDERSTATE_ALPHABLENDENABLE: m->alphaBlend = (value != 0); break;
        case D3DRENDERSTATE_SRCBLEND:         m->srcBlend = value; break;
        case D3DRENDERSTATE_DESTBLEND:        m->dstBlend = value; break;
        case D3DRENDERSTATE_ALPHATESTENABLE:  m->alphaTest = (value != 0); break;
        case D3DRENDERSTATE_ALPHAFUNC:        m->alphaFunc = value; break;
        case D3DRENDERSTATE_ALPHAREF:         m->alphaRef = value; break;
        case D3DRENDERSTATE_FOGENABLE:        m->fogEnable = (value != 0); break;
        case D3DRENDERSTATE_FOGCOLOR:         m->fogColor = value; break;
        case D3DRENDERSTATE_FOGSTART:         m->fogStart = *(float*)&value; break;
        case D3DRENDERSTATE_FOGEND:           m->fogEnd = *(float*)&value; break;
        case D3DRENDERSTATE_LIGHTING:         m->lighting = (value != 0); break;
        case D3DRENDERSTATE_AMBIENT:          m->ambient = value; break;
        case D3DRENDERSTATE_TEXTUREFACTOR:    m->tFactor = value; break;
        case D3DRENDERSTATE_CULLMODE:         m->cullMode = value; break;
        case D3DRENDERSTATE_ZBIAS:            m->zBias = (int)value; break;
        default: break;  // dither, specular, shade mode, wrap0, fill mode: ignored
    }
    return S_OK;
}

HRESULT IDirect3DDevice7::GetRenderState(D3DRENDERSTATETYPE state, LPDWORD out)
{
    if (out == NULL) return E_INVALIDARG;
    switch (state)
    {
        case D3DRENDERSTATE_ZENABLE:          *out = (DWORD)m->zEnable; break;
        case D3DRENDERSTATE_ZWRITEENABLE:     *out = (DWORD)m->zWrite; break;
        case D3DRENDERSTATE_ALPHABLENDENABLE: *out = (DWORD)m->alphaBlend; break;
        case D3DRENDERSTATE_FOGENABLE:        *out = (DWORD)m->fogEnable; break;
        case D3DRENDERSTATE_LIGHTING:         *out = (DWORD)m->lighting; break;
        case D3DRENDERSTATE_AMBIENT:          *out = m->ambient; break;
        default: *out = 0; break;
    }
    return S_OK;
}

// ------------------------------------------------------------ texture state

HRESULT IDirect3DDevice7::SetTexture(DWORD stage, LPDIRECTDRAWSURFACE7 tex)
{
    if (stage >= 2) return S_OK;
    m->stage[stage].texture = tex;
    return S_OK;
}

HRESULT IDirect3DDevice7::SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE st, DWORD v)
{
    if (stage >= 2) return S_OK;
    TexStage& s = m->stage[stage];
    switch (st)
    {
        case D3DTSS_COLOROP:   s.colorOp = v; break;
        case D3DTSS_COLORARG1: s.colorArg1 = v; break;
        case D3DTSS_COLORARG2: s.colorArg2 = v; break;
        case D3DTSS_ALPHAOP:   s.alphaOp = v; break;
        case D3DTSS_ALPHAARG1: s.alphaArg1 = v; break;
        case D3DTSS_ADDRESS:   s.address = v; break;
        case D3DTSS_ADDRESSU:  s.address = v; break;
        case D3DTSS_ADDRESSV:  s.address = v; break;
        case D3DTSS_MAGFILTER: s.magFilter = v; break;
        case D3DTSS_MINFILTER: s.minFilter = v; break;
        case D3DTSS_TEXCOORDINDEX: s.texCoordIndex = v; break;
        default: break;
    }
    return S_OK;
}

// ------------------------------------------------------------------ drawing

static GLenum BlendToGL(DWORD b, BOOL bSrc)
{
    switch (b)
    {
        case D3DBLEND_ZERO:         return GL_ZERO;
        case D3DBLEND_ONE:          return GL_ONE;
        case D3DBLEND_SRCCOLOR:     return GL_SRC_COLOR;
        case D3DBLEND_INVSRCCOLOR:  return GL_ONE_MINUS_SRC_COLOR;
        case D3DBLEND_SRCALPHA:     return GL_SRC_ALPHA;
        case D3DBLEND_INVSRCALPHA:  return GL_ONE_MINUS_SRC_ALPHA;
        case D3DBLEND_DESTALPHA:    return GL_DST_ALPHA;
        case D3DBLEND_INVDESTALPHA: return GL_ONE_MINUS_DST_ALPHA;
        case D3DBLEND_DESTCOLOR:    return GL_DST_COLOR;
        case D3DBLEND_INVDESTCOLOR: return GL_ONE_MINUS_DST_COLOR;
        case D3DBLEND_SRCALPHASAT:  return GL_SRC_ALPHA_SATURATE;
        case D3DBLEND_BOTHSRCALPHA:    return bSrc ? GL_SRC_ALPHA : GL_ONE_MINUS_SRC_ALPHA;
        case D3DBLEND_BOTHINVSRCALPHA: return bSrc ? GL_ONE_MINUS_SRC_ALPHA : GL_SRC_ALPHA;
    }
    return bSrc ? GL_ONE : GL_ZERO;
}

static GLenum CmpToGL(DWORD f)
{
    switch (f)
    {
        case D3DCMP_NEVER:        return GL_NEVER;
        case D3DCMP_LESS:         return GL_LESS;
        case D3DCMP_EQUAL:        return GL_EQUAL;
        case D3DCMP_LESSEQUAL:    return GL_LEQUAL;
        case D3DCMP_GREATER:      return GL_GREATER;
        case D3DCMP_NOTEQUAL:     return GL_NOTEQUAL;
        case D3DCMP_GREATEREQUAL: return GL_GEQUAL;
        case D3DCMP_ALWAYS:       return GL_ALWAYS;
    }
    return GL_LEQUAL;
}

static void ApplyTexParams(const TexStage& s)
{
    GLint wrap = (s.address == D3DTADDRESS_CLAMP) ? GL_CLAMP_TO_EDGE :
                 (s.address == D3DTADDRESS_MIRROR) ? GL_MIRRORED_REPEAT : GL_REPEAT;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    (s.magFilter == D3DTFG_POINT) ? GL_NEAREST : GL_LINEAR);
    // every texture is uploaded as power-of-two with mipmaps (see BindGL)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    (s.minFilter == D3DTFN_POINT) ? GL_NEAREST_MIPMAP_NEAREST
                                                  : GL_LINEAR_MIPMAP_LINEAR);
}

void IDirect3DDevice7::ApplyState(DWORD fvf)
{
    Impl* d = m;

    // depth
    if (d->zEnable) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    glDepthMask(d->zWrite ? GL_TRUE : GL_FALSE);
    glDepthFunc(CmpToGL(d->zFunc));

    if (d->zBias != 0)
    {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-(float)d->zBias, -(float)d->zBias * 2.0f);
    }
    else
    {
        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    // blending
    if (d->alphaBlend)
    {
        glEnable(GL_BLEND);
        glBlendFunc(BlendToGL(d->srcBlend, TRUE), BlendToGL(d->dstBlend, FALSE));
    }
    else glDisable(GL_BLEND);

    // culling
    if (d->cullMode == D3DCULL_NONE) glDisable(GL_CULL_FACE);
    else
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(d->cullMode == D3DCULL_CCW ? GL_CW : GL_CCW);
    }

    // matrices
    UpdateMatrices();
    glUniformMatrix4fv(d->uMvp, 1, GL_FALSE, (const GLfloat*)d->mvp.m);
    glUniformMatrix4fv(d->uWorld, 1, GL_FALSE, (const GLfloat*)d->world.m);
    glUniformMatrix4fv(d->uView, 1, GL_FALSE, (const GLfloat*)d->view.m);

    // lighting: only when the FVF carries normals
    BOOL bLit = d->lighting && (fvf & D3DFVF_NORMAL) != 0;
    glUniform1i(d->uLighting, bLit ? 1 : 0);
    glUniform1i(d->uUseVtxColor, (fvf & D3DFVF_DIFFUSE) ? 1 : 0);

    if (bLit)
    {
        float c[4];
        glUniform4f(d->uMatDiffuse, d->material.diffuse.r, d->material.diffuse.g,
                    d->material.diffuse.b, d->material.diffuse.a);
        glUniform4f(d->uMatAmbient, d->material.ambient.r, d->material.ambient.g,
                    d->material.ambient.b, d->material.ambient.a);
        glUniform4f(d->uMatEmissive, d->material.emissive.r, d->material.emissive.g,
                    d->material.emissive.b, d->material.emissive.a);
        ColorToFloats(d->ambient, c);
        glUniform4f(d->uGlobalAmbient, c[0], c[1], c[2], c[3]);

        float pos[MAX_GPU_LIGHTS][4], dir[MAX_GPU_LIGHTS][4];
        float dif[MAX_GPU_LIGHTS][4], amb[MAX_GPU_LIGHTS][4];
        float att[MAX_GPU_LIGHTS][4], spot[MAX_GPU_LIGHTS][2];
        int count = 0;
        for (int i = 0; i < MAX_LIGHTS && count < MAX_GPU_LIGHTS; i++)
        {
            if (!d->lights[i].bEnabled || !d->lights[i].bSet) continue;
            const D3DLIGHT7& L = d->lights[i].light;
            if (L.dltType == D3DLIGHT_DIRECTIONAL)
            {
                pos[count][0] = -L.dvDirection.x;
                pos[count][1] = -L.dvDirection.y;
                pos[count][2] = -L.dvDirection.z;
                pos[count][3] = 0.0f;
                dir[count][0] = dir[count][1] = dir[count][2] = 0.0f;
                dir[count][3] = 0.0f;
            }
            else
            {
                pos[count][0] = L.dvPosition.x;
                pos[count][1] = L.dvPosition.y;
                pos[count][2] = L.dvPosition.z;
                pos[count][3] = 1.0f;
                if (L.dltType == D3DLIGHT_SPOT)
                {
                    dir[count][0] = L.dvDirection.x;
                    dir[count][1] = L.dvDirection.y;
                    dir[count][2] = L.dvDirection.z;
                    dir[count][3] = cosf(L.dvPhi * 0.5f);
                }
                else
                {
                    dir[count][0] = dir[count][1] = dir[count][2] = 0.0f;
                    dir[count][3] = 0.0f;
                }
            }
            dif[count][0] = L.dcvDiffuse.r; dif[count][1] = L.dcvDiffuse.g;
            dif[count][2] = L.dcvDiffuse.b; dif[count][3] = L.dcvDiffuse.a;
            amb[count][0] = L.dcvAmbient.r; amb[count][1] = L.dcvAmbient.g;
            amb[count][2] = L.dcvAmbient.b; amb[count][3] = L.dcvAmbient.a;
            att[count][0] = L.dvAttenuation0;
            att[count][1] = L.dvAttenuation1;
            att[count][2] = L.dvAttenuation2;
            att[count][3] = (L.dvRange > 0.0f) ? L.dvRange : 1.0e9f;
            spot[count][0] = cosf(L.dvTheta * 0.5f);
            spot[count][1] = L.dvFalloff;
            count++;
        }
        glUniform1i(d->uLightCount, count);
        if (count > 0)
        {
            glUniform4fv(d->uLightPos, count, &pos[0][0]);
            glUniform4fv(d->uLightDir, count, &dir[0][0]);
            glUniform4fv(d->uLightDiffuse, count, &dif[0][0]);
            glUniform4fv(d->uLightAmbient, count, &amb[0][0]);
            glUniform4fv(d->uLightAtten, count, &att[0][0]);
            // u_lightSpot is a vec2 array
            float spotFlat[MAX_GPU_LIGHTS*2];
            for (int i = 0; i < count; i++)
            {
                spotFlat[i*2+0] = spot[i][0];
                spotFlat[i*2+1] = spot[i][1];
            }
            glUniform2fv(d->uLightSpot, count, spotFlat);
        }
    }

    // fog
    glUniform1i(d->uFogEnableV, d->fogEnable ? 1 : 0);
    if (d->fogEnable)
    {
        float c[4];
        ColorToFloats(d->fogColor, c);
        glUniform4f(d->uFogColor, c[0], c[1], c[2], c[3]);
        glUniform2f(d->uFogRange, d->fogStart, d->fogEnd);
    }

    // texture stages
    for (int i = 0; i < 2; i++)
    {
        TexStage& s = d->stage[i];
        BOOL bUse = (s.colorOp != D3DTOP_DISABLE) && (s.texture != NULL);
        glActiveTexture(i == 0 ? GL_TEXTURE0 : GL_TEXTURE1);
        if (bUse)
        {
            s.texture->BindGL();
            ApplyTexParams(s);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        if (i == 0)
        {
            glUniform1i(d->uS0op,   bUse ? (GLint)s.colorOp : 1);
            glUniform1i(d->uS0arg1, (GLint)(s.colorArg1 & D3DTA_SELECTMASK));
            glUniform1i(d->uS0arg2, (GLint)(s.colorArg2 & D3DTA_SELECTMASK));
            glUniform1i(d->uS0aop,  bUse ? (GLint)s.alphaOp : 1);
            glUniform1i(d->uS0aarg1,(GLint)(s.alphaArg1 & D3DTA_SELECTMASK));
        }
        else
        {
            glUniform1i(d->uS1op,   bUse ? (GLint)s.colorOp : 1);
            glUniform1i(d->uS1arg1, (GLint)(s.colorArg1 & D3DTA_SELECTMASK));
            glUniform1i(d->uS1arg2, (GLint)(s.colorArg2 & D3DTA_SELECTMASK));
            glUniform1i(d->uS1uv,   (GLint)(s.texCoordIndex >= 1 ? 1 : 0));
        }
    }
    glActiveTexture(GL_TEXTURE0);

    // tfactor / alpha test
    float tf[4];
    ColorToFloats(d->tFactor, tf);
    glUniform4f(d->uTfactor, tf[0], tf[1], tf[2], tf[3]);
    glUniform1i(d->uAlphaTest, d->alphaTest ? (GLint)d->alphaFunc : 0);
    glUniform1f(d->uAlphaRef, (float)d->alphaRef / 255.0f);
}

// FVF layout: returns stride; fills offsets (-1 if absent)
static int ParseFVF(DWORD fvf, int* offNormal, int* offColor, int* offUV0, int* offUV1)
{
    int off = 12;   // XYZ position
    *offNormal = *offColor = *offUV0 = *offUV1 = -1;
    if (fvf & D3DFVF_RESERVED1) off += 4;
    if (fvf & D3DFVF_NORMAL) { *offNormal = off; off += 12; }
    if (fvf & D3DFVF_DIFFUSE) { *offColor = off; off += 4; }
    if (fvf & D3DFVF_SPECULAR) { off += 4; }
    int texCount = (int)((fvf & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT);
    if (texCount >= 1) { *offUV0 = off; off += 8; }
    if (texCount >= 2) { *offUV1 = off; off += 8; }
    return off;
}

HRESULT IDirect3DDevice7::DrawPrimitive(D3DPRIMITIVETYPE type, DWORD fvf,
                                        LPVOID vertices, DWORD count, DWORD flags)
{
    (void)flags;
    if (vertices == NULL || count == 0) return S_OK;

    glUseProgram(m->program);
    ApplyState(fvf);

    int offNormal, offColor, offUV0, offUV1;
    int stride = ParseFVF(fvf, &offNormal, &offColor, &offUV0, &offUV1);
    const unsigned char* base = (const unsigned char*)vertices;

    glEnableVertexAttribArray(ATTR_POS);
    glVertexAttribPointer(ATTR_POS, 3, GL_FLOAT, GL_FALSE, stride, base);

    if (offNormal >= 0)
    {
        glEnableVertexAttribArray(ATTR_NORMAL);
        glVertexAttribPointer(ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, stride, base + offNormal);
    }
    else glDisableVertexAttribArray(ATTR_NORMAL);

    if (offColor >= 0)
    {
        glEnableVertexAttribArray(ATTR_COLOR);
        glVertexAttribPointer(ATTR_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, base + offColor);
    }
    else glDisableVertexAttribArray(ATTR_COLOR);

    if (offUV0 >= 0)
    {
        glEnableVertexAttribArray(ATTR_UV0);
        glVertexAttribPointer(ATTR_UV0, 2, GL_FLOAT, GL_FALSE, stride, base + offUV0);
    }
    else glDisableVertexAttribArray(ATTR_UV0);

    if (offUV1 >= 0)
    {
        glEnableVertexAttribArray(ATTR_UV1);
        glVertexAttribPointer(ATTR_UV1, 2, GL_FLOAT, GL_FALSE, stride, base + offUV1);
    }
    else glDisableVertexAttribArray(ATTR_UV1);

    GLenum mode = GL_TRIANGLES;
    switch (type)
    {
        case D3DPT_TRIANGLELIST:  mode = GL_TRIANGLES; break;
        case D3DPT_TRIANGLESTRIP: mode = GL_TRIANGLE_STRIP; break;
        case D3DPT_TRIANGLEFAN:   mode = GL_TRIANGLE_FAN; break;
        default: return S_OK;
    }
    glDrawArrays(mode, 0, (GLsizei)count);
    return S_OK;
}

HRESULT IDirect3DDevice7::DrawIndexedPrimitive(D3DPRIMITIVETYPE type, DWORD fvf,
                                               LPVOID vertices, DWORD vcount,
                                               LPWORD indices, DWORD icount, DWORD flags)
{
    (void)vcount;
    if (vertices == NULL || indices == NULL || icount == 0) return S_OK;
    (void)flags;

    glUseProgram(m->program);
    ApplyState(fvf);

    int offNormal, offColor, offUV0, offUV1;
    int stride = ParseFVF(fvf, &offNormal, &offColor, &offUV0, &offUV1);
    const unsigned char* base = (const unsigned char*)vertices;

    glEnableVertexAttribArray(ATTR_POS);
    glVertexAttribPointer(ATTR_POS, 3, GL_FLOAT, GL_FALSE, stride, base);
    if (offNormal >= 0)
    {
        glEnableVertexAttribArray(ATTR_NORMAL);
        glVertexAttribPointer(ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, stride, base + offNormal);
    }
    else glDisableVertexAttribArray(ATTR_NORMAL);
    if (offColor >= 0)
    {
        glEnableVertexAttribArray(ATTR_COLOR);
        glVertexAttribPointer(ATTR_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, base + offColor);
    }
    else glDisableVertexAttribArray(ATTR_COLOR);
    if (offUV0 >= 0)
    {
        glEnableVertexAttribArray(ATTR_UV0);
        glVertexAttribPointer(ATTR_UV0, 2, GL_FLOAT, GL_FALSE, stride, base + offUV0);
    }
    else glDisableVertexAttribArray(ATTR_UV0);
    if (offUV1 >= 0)
    {
        glEnableVertexAttribArray(ATTR_UV1);
        glVertexAttribPointer(ATTR_UV1, 2, GL_FLOAT, GL_FALSE, stride, base + offUV1);
    }
    else glDisableVertexAttribArray(ATTR_UV1);

    GLenum mode = (type == D3DPT_TRIANGLESTRIP) ? GL_TRIANGLE_STRIP :
                  (type == D3DPT_TRIANGLEFAN)   ? GL_TRIANGLE_FAN : GL_TRIANGLES;
    glDrawElements(mode, (GLsizei)icount, GL_UNSIGNED_SHORT, indices);
    return S_OK;
}

// -------------------------------------------------------------- visibility

// D3DSTATUS bits: any CLIPINTERSECTION* bit set means the sphere is fully
// outside that frustum plane. The engine only tests against
// D3DSTATUS_CLIPINTERSECTIONALL (defined in the d3d.h shim as 0xFFF000).
HRESULT IDirect3DDevice7::ComputeSphereVisibility(LPD3DVECTOR centers, LPD3DVALUE radii,
                                                  DWORD n, DWORD flags, LPDWORD results)
{
    (void)flags;
    if (centers == NULL || radii == NULL || results == NULL) return E_INVALIDARG;

    UpdateMatrices();
    const D3DMATRIX& M = m->mvp;

    for (DWORD s = 0; s < n; s++)
    {
        const D3DVECTOR& c = centers[s];
        float r = radii[s];

        // transform center to clip space (row-vector: v * M)
        float x = c.x*M._11 + c.y*M._21 + c.z*M._31 + M._41;
        float y = c.x*M._12 + c.y*M._22 + c.z*M._32 + M._42;
        float z = c.x*M._13 + c.y*M._23 + c.z*M._33 + M._43;
        float w = c.x*M._14 + c.y*M._24 + c.z*M._34 + M._44;

        // conservative radius scale: length of world->clip scale is approximated
        // by the largest row norm of the matrix
        float sx = sqrtf(M._11*M._11 + M._21*M._21 + M._31*M._31);
        float sy = sqrtf(M._12*M._12 + M._22*M._22 + M._32*M._32);
        float sw = sqrtf(M._14*M._14 + M._24*M._24 + M._34*M._34);
        float scale = sx > sy ? sx : sy;
        if (sw > scale) scale = sw;
        float rc = r * scale;

        DWORD out = 0;
        // fully outside if outside any plane: |x| > w + rc etc. (D3D z in [0,w])
        if (x < -w - rc) out |= 0x001000;
        if (x >  w + rc) out |= 0x002000;
        if (y < -w - rc) out |= 0x004000;
        if (y >  w + rc) out |= 0x008000;
        if (z <  0 - rc) out |= 0x010000;
        if (z >  w + rc) out |= 0x020000;
        results[s] = out;
    }
    return S_OK;
}

// ------------------------------------------------------------------- misc

HRESULT IDirect3DDevice7::SetRenderTarget(LPDIRECTDRAWSURFACE7 target, DWORD flags)
{
    (void)target; (void)flags;
    return S_OK;    // offscreen render targets not used by the game paths we keep
}

HRESULT IDirect3DDevice7::GetCaps(LPD3DDEVICEDESC7 desc)
{
    if (desc == NULL) return E_INVALIDARG;
    memset(desc, 0, sizeof(D3DDEVICEDESC7));
    GLint maxTex = 2048;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTex);
    desc->dwMaxTextureWidth = (DWORD)maxTex;
    desc->dwMaxTextureHeight = (DWORD)maxTex;
    desc->wMaxSimultaneousTextures = 2;
    desc->dwMaxActiveLights = MAX_LIGHTS;
    desc->dpcTriCaps.dwTextureBlendCaps = D3DPTBLENDCAPS_ADD;
    return S_OK;
}

HRESULT IDirect3DDevice7::ValidateDevice(LPDWORD passes)
{
    if (passes != NULL) *passes = 1;
    return S_OK;
}
