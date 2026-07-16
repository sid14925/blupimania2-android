// ddraw.h — DirectDraw compatibility shim for the BlupiMania 2 SDL/OpenGL port.
// IDirectDrawSurface7 is emulated as a CPU pixel buffer (32-bit ARGB) plus an
// OpenGL texture; Lock/Unlock give the game direct pixel access (used for the
// dynamic ground-spot/shadow textures in d3dengine.cpp).

#ifndef _PORT_DDRAW_H_
#define _PORT_DDRAW_H_

#include <windows.h>

typedef struct _DDPIXELFORMAT
{
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwFourCC;
    union { DWORD dwRGBBitCount; DWORD dwYUVBitCount; DWORD dwAlphaBitDepth; };
    union { DWORD dwRBitMask; DWORD dwYBitMask; };
    union { DWORD dwGBitMask; DWORD dwUBitMask; };
    union { DWORD dwBBitMask; DWORD dwVBitMask; };
    union { DWORD dwRGBAlphaBitMask; DWORD dwYUVAlphaBitMask; };
} DDPIXELFORMAT, *LPDDPIXELFORMAT;

typedef struct _DDSCAPS2
{
    DWORD dwCaps;
    DWORD dwCaps2;
    DWORD dwCaps3;
    DWORD dwCaps4;
} DDSCAPS2, *LPDDSCAPS2;

typedef struct _DDCOLORKEY
{
    DWORD dwColorSpaceLowValue;
    DWORD dwColorSpaceHighValue;
} DDCOLORKEY, *LPDDCOLORKEY;

typedef struct _DDSURFACEDESC2
{
    DWORD         dwSize;
    DWORD         dwFlags;
    DWORD         dwHeight;
    DWORD         dwWidth;
    union { LONG lPitch; DWORD dwLinearSize; };
    DWORD         dwBackBufferCount;
    union { DWORD dwMipMapCount; DWORD dwRefreshRate; };
    DWORD         dwAlphaBitDepth;
    DWORD         dwReserved;
    LPVOID        lpSurface;
    DDCOLORKEY    ddckCKDestOverlay;
    DDCOLORKEY    ddckCKDestBlt;
    DDCOLORKEY    ddckCKSrcOverlay;
    DDCOLORKEY    ddckCKSrcBlt;
    DDPIXELFORMAT ddpfPixelFormat;
    DDSCAPS2      ddsCaps;
    DWORD         dwTextureStage;
} DDSURFACEDESC2, *LPDDSURFACEDESC2;

// dwFlags bits
#define DDSD_CAPS            0x00000001
#define DDSD_HEIGHT          0x00000002
#define DDSD_WIDTH           0x00000004
#define DDSD_PITCH           0x00000008
#define DDSD_PIXELFORMAT     0x00001000
#define DDSD_MIPMAPCOUNT     0x00020000
#define DDSD_TEXTURESTAGE    0x00100000

// pixel format flags
#define DDPF_ALPHAPIXELS     0x00000001
#define DDPF_RGB             0x00000040

// caps
#define DDSCAPS_TEXTURE      0x00001000
#define DDSCAPS_MIPMAP       0x00400000
#define DDSCAPS_SYSTEMMEMORY 0x00000800
#define DDSCAPS_VIDEOMEMORY  0x00004000
#define DDSCAPS2_TEXTUREMANAGE 0x00000002

// Lock flags
#define DDLOCK_WAIT          0x00000001
#define DDLOCK_READONLY      0x00000010
#define DDLOCK_WRITEONLY     0x00000020
#define DDLOCK_SURFACEMEMORYPTR 0x00000000

#define DD_OK                S_OK

// Emulated DirectDraw surface: 32-bit ARGB pixel buffer + GL texture.
class IDirect3DDevice7;

class IDirectDrawSurface7
{
public:
    IDirectDrawSurface7(int width, int height, BOOL bAlpha);
    ~IDirectDrawSurface7();

    // D3D7-style API used by game code
    HRESULT Lock(LPRECT lpDestRect, LPDDSURFACEDESC2 lpDDSurfaceDesc,
                 DWORD dwFlags, HANDLE hEvent);
    HRESULT Unlock(LPRECT lpRect);
    HRESULT GetSurfaceDesc(LPDDSURFACEDESC2 lpDDSurfaceDesc);
    ULONG   AddRef();
    ULONG   Release();

    // port-internal API (texture loader / GL device)
    void    FillDesc(DDSURFACEDESC2* d);
    DWORD*  Pixels()        { return m_pixels; }
    int     Width() const   { return m_width; }
    int     Height() const  { return m_height; }
    BOOL    HasAlpha() const { return m_bAlpha; }
    void    MarkDirty()     { m_bDirty = TRUE; }

    // binds the GL texture, (re)uploading pixels if dirty; returns GL id
    unsigned int BindGL();
    void    InvalidateGL();     // GL context lost — drop the texture object

private:
    int          m_width;
    int          m_height;
    BOOL         m_bAlpha;
    BOOL         m_bDirty;
    DWORD*       m_pixels;      // ARGB (A in high byte), row-major, w*h
    unsigned int m_glTexture;   // 0 if not yet created
    int          m_refCount;
};

typedef IDirectDrawSurface7* LPDIRECTDRAWSURFACE7;

// legacy structs referenced by d3dapp.h / framework headers
typedef struct _DDCAPS
{
    DWORD dwSize;
    DWORD dwVidMemTotal;
    DWORD dwVidMemFree;
} DDCAPS, *LPDDCAPS;

#endif // _PORT_DDRAW_H_
