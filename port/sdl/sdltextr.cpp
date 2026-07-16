// sdltextr.cpp — D3DTextr_* texture management for the port
// (replaces src/d3dtextr.cpp). Textures load from the game's metafiles
// (blupimania1.dat) exactly like the original, but decode into the port's
// IDirectDrawSurface7 (ARGB pixel buffer + GL texture).

#define STRICT
#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <stdio.h>

#include "D3DTextr.h"
#include "misc.h"
#include "metafile.h"

extern CMetaFile g_metafile;

static TCHAR g_strTexturePath[512] = "";
static BOOL  g_bDebugMode = FALSE;

//-----------------------------------------------------------------------------
// texture container list
//-----------------------------------------------------------------------------

struct TextureContainer
{
    TextureContainer* m_pNext;

    TCHAR   m_strName[80];
    DWORD   m_dwWidth;
    DWORD   m_dwHeight;
    DWORD   m_dwStage;
    DWORD   m_dwBPP;
    DWORD   m_dwFlags;
    BOOL    m_bHasAlpha;

    DWORD*  m_pRGBAData;    // decoded image: (r<<24)|(g<<16)|(b<<8)|a
    IDirectDrawSurface7* m_pddsSurface;

    TextureContainer(TCHAR* strName, DWORD dwStage, DWORD dwFlags);
    ~TextureContainer();

    HRESULT LoadImageData();
    HRESULT LoadTargaFile(TCHAR* strMetaname, TCHAR* strFilename);
    HRESULT LoadBitmapFile(TCHAR* strPathname);
    HRESULT Restore(LPDIRECT3DDEVICE7 pd3dDevice);
};

static TextureContainer* g_ptcTextureList = NULL;

static TextureContainer* FindTexture(TCHAR* strName)
{
    for (TextureContainer* ptc = g_ptcTextureList; ptc; ptc = ptc->m_pNext)
    {
        if (lstrcmpi(strName, ptc->m_strName) == 0) return ptc;
    }
    return NULL;
}

TextureContainer::TextureContainer(TCHAR* strName, DWORD dwStage, DWORD dwFlags)
{
    lstrcpyn(m_strName, strName, sizeof(m_strName));
    m_dwWidth  = 0;
    m_dwHeight = 0;
    m_dwStage  = dwStage;
    m_dwBPP    = 0;
    m_dwFlags  = dwFlags;
    m_bHasAlpha = FALSE;
    m_pRGBAData = NULL;
    m_pddsSurface = NULL;

    m_pNext = g_ptcTextureList;
    g_ptcTextureList = this;
}

TextureContainer::~TextureContainer()
{
    if (m_pddsSurface != NULL) m_pddsSurface->Release();
    delete[] m_pRGBAData;

    if (g_ptcTextureList == this)
    {
        g_ptcTextureList = m_pNext;
    }
    else
    {
        for (TextureContainer* ptc = g_ptcTextureList; ptc; ptc = ptc->m_pNext)
            if (ptc->m_pNext == this)
                ptc->m_pNext = m_pNext;
    }
}

//-----------------------------------------------------------------------------
// image loading
//-----------------------------------------------------------------------------

HRESULT TextureContainer::LoadImageData()
{
    TCHAR* strExtension;
    TCHAR  strMetaname[256];
    TCHAR  strFilename[256];
    TCHAR  strSuppl[10];

    if (g_bDebugMode)
    {
        if (strrchr(m_strName, '\\') == 0)
        {
            lstrcpy(strMetaname, "");
            lstrcpy(strSuppl, "");
            lstrcpy(strFilename, g_strTexturePath);
            lstrcat(strFilename, m_strName);
        }
        else
        {
            lstrcpy(strMetaname, "");
            lstrcpy(strSuppl, "");
            lstrcpy(strFilename, m_strName);
        }
    }
    else
    {
        if (strrchr(m_strName, '\\') == 0)
        {
            lstrcpy(strMetaname, "blupimania1.dat");
            lstrcpy(strSuppl, "b");
            lstrcpy(strFilename, m_strName);
        }
        else
        {
            lstrcpy(strMetaname, "");
            lstrcpy(strSuppl, "");
            lstrcpy(strFilename, m_strName);
        }
    }

    if (!g_metafile.IsExist(strMetaname, strFilename, strSuppl))
    {
        return E_FAIL;
    }

    if (NULL == (strExtension = strrchr(m_strName, '.')))
    {
        return E_FAIL;
    }

    if (strMetaname[0] == 0 && !lstrcmpi(strExtension, ".bmp"))
    {
        return LoadBitmapFile(strFilename);
    }

    if (!lstrcmpi(strExtension, ".tga"))
    {
        return LoadTargaFile(strMetaname, strFilename);
    }

    return E_FAIL;
}

// Faithful port of the original TGA loader, including its use of
// GetByte/GetWord on the metafile stream (the encrypted .dat requires the
// exact same read pattern to keep the decode position in sync).
HRESULT TextureContainer::LoadTargaFile(TCHAR* strMetaname, TCHAR* strFilename)
{
    if (g_metafile.Open(strMetaname, strFilename, (char*)"b") != 0)
        return E_FAIL;

#pragma pack(push, 1)
    struct TargaHeader
    {
        BYTE IDLength;
        BYTE ColormapType;
        BYTE ImageType;
        BYTE ColormapSpecification[5];
        WORD XOrigin;
        WORD YOrigin;
        WORD ImageWidth;
        WORD ImageHeight;
        BYTE PixelDepth;
        BYTE ImageDescriptor;
    } tga;
#pragma pack(pop)

    g_metafile.Read(&tga, sizeof(TargaHeader));

    if ((0 != tga.ColormapType) ||
        (tga.ImageType != 10 && tga.ImageType != 2))
    {
        g_metafile.Close();
        return E_FAIL;
    }

    if (tga.IDLength)
    {
        g_metafile.Seek(tga.IDLength);
    }

    m_dwWidth  = tga.ImageWidth;
    m_dwHeight = tga.ImageHeight;
    m_dwBPP    = tga.PixelDepth;
    m_pRGBAData = new DWORD[m_dwWidth*m_dwHeight];

    if (m_pRGBAData == NULL)
    {
        g_metafile.Close();
        return E_FAIL;
    }

    for (DWORD y = 0; y < m_dwHeight; y++)
    {
        DWORD dwOffset = y*m_dwWidth;

        if (0 == (tga.ImageDescriptor & 0x0010))
            dwOffset = (m_dwHeight-y-1)*m_dwWidth;

        for (DWORD x = 0; x < m_dwWidth; )
        {
            if (tga.ImageType == 10)
            {
                BYTE PacketInfo = (BYTE)g_metafile.GetByte();
                WORD PacketType = 0x80 & PacketInfo;
                WORD PixelCount = (0x007f & PacketInfo) + 1;

                if (PacketType)
                {
                    DWORD b = (DWORD)g_metafile.GetWord();
                    DWORD g = (DWORD)g_metafile.GetWord();
                    DWORD r = (DWORD)g_metafile.GetWord();
                    DWORD a = 0xff;
                    if (m_dwBPP == 32)
                        a = (DWORD)g_metafile.GetWord();

                    while (PixelCount--)
                    {
                        if (x >= m_dwWidth) break;
                        m_pRGBAData[dwOffset+x] = (r<<24L)+(g<<16L)+(b<<8L)+(a);
                        x++;
                    }
                }
                else
                {
                    while (PixelCount--)
                    {
                        if (x >= m_dwWidth) break;
                        BYTE b = (BYTE)g_metafile.GetByte();
                        BYTE g = (BYTE)g_metafile.GetByte();
                        BYTE r = (BYTE)g_metafile.GetByte();
                        BYTE a = 0xff;
                        if (m_dwBPP == 32)
                            a = (BYTE)g_metafile.GetByte();

                        m_pRGBAData[dwOffset+x] = ((DWORD)r<<24L)+((DWORD)g<<16L)+((DWORD)b<<8L)+((DWORD)a);
                        x++;
                    }
                }
            }
            else
            {
                BYTE b = (BYTE)g_metafile.GetByte();
                BYTE g = (BYTE)g_metafile.GetByte();
                BYTE r = (BYTE)g_metafile.GetByte();
                BYTE a = 0xff;
                if (m_dwBPP == 32)
                    a = (BYTE)g_metafile.GetByte();

                m_pRGBAData[dwOffset+x] = ((DWORD)r<<24L)+((DWORD)g<<16L)+((DWORD)b<<8L)+((DWORD)a);
                x++;
            }
        }
    }

    g_metafile.Close();

    for (DWORD i = 0; i < (m_dwWidth*m_dwHeight); i++)
    {
        if ((m_pRGBAData[i] & 0x000000ff) != 0xff)
        {
            m_bHasAlpha = TRUE;
            break;
        }
    }

    return S_OK;
}

// minimal uncompressed 24/32-bit BMP reader for -debug mode file textures
HRESULT TextureContainer::LoadBitmapFile(TCHAR* strPathname)
{
    FILE* f = fopen(strPathname, "rb");
    if (f == NULL) return E_FAIL;

    unsigned char hdr[54];
    if (fread(hdr, 1, 54, f) != 54 || hdr[0] != 'B' || hdr[1] != 'M')
    {
        fclose(f);
        return E_FAIL;
    }

    int dataOffset = *(int*)(hdr+10);
    int width      = *(int*)(hdr+18);
    int height     = *(int*)(hdr+22);
    int bpp        = *(short*)(hdr+28);
    int compress   = *(int*)(hdr+30);

    if (compress != 0 || (bpp != 24 && bpp != 32) || width <= 0)
    {
        fclose(f);
        return E_FAIL;
    }

    BOOL bTopDown = (height < 0);
    if (bTopDown) height = -height;

    m_dwWidth  = (DWORD)width;
    m_dwHeight = (DWORD)height;
    m_dwBPP    = (DWORD)bpp;
    m_pRGBAData = new DWORD[m_dwWidth*m_dwHeight];

    fseek(f, dataOffset, SEEK_SET);
    int rowSize = ((width*(bpp/8)) + 3) & ~3;
    unsigned char* row = (unsigned char*)malloc((size_t)rowSize);

    for (int y = 0; y < height; y++)
    {
        if (fread(row, 1, (size_t)rowSize, f) != (size_t)rowSize) break;
        int dstY = bTopDown ? y : (height-1-y);
        DWORD* dst = m_pRGBAData + (size_t)dstY*width;
        for (int x = 0; x < width; x++)
        {
            unsigned char b = row[x*(bpp/8)+0];
            unsigned char g = row[x*(bpp/8)+1];
            unsigned char r = row[x*(bpp/8)+2];
            unsigned char a = (bpp == 32) ? row[x*4+3] : 0xff;
            dst[x] = ((DWORD)r<<24)|((DWORD)g<<16)|((DWORD)b<<8)|a;
        }
    }
    free(row);
    fclose(f);
    return S_OK;
}

//-----------------------------------------------------------------------------
// surface creation
//-----------------------------------------------------------------------------

HRESULT TextureContainer::Restore(LPDIRECT3DDEVICE7 pd3dDevice)
{
    (void)pd3dDevice;
    if (m_pRGBAData == NULL) return E_FAIL;

    if (m_pddsSurface == NULL)
    {
        BOOL bAlpha = m_bHasAlpha ||
                      (m_dwFlags & (D3DTEXTR_TRANSPARENTWHITE|
                                    D3DTEXTR_TRANSPARENTBLACK|
                                    D3DTEXTR_CREATEWITHALPHA)) != 0;
        m_pddsSurface = new IDirectDrawSurface7((int)m_dwWidth, (int)m_dwHeight, bAlpha);
    }

    DWORD* dst = m_pddsSurface->Pixels();
    for (DWORD i = 0; i < m_dwWidth*m_dwHeight; i++)
    {
        DWORD s = m_pRGBAData[i];   // (r<<24)|(g<<16)|(b<<8)|a
        DWORD r = (s >> 24) & 0xff;
        DWORD g = (s >> 16) & 0xff;
        DWORD b = (s >> 8) & 0xff;
        DWORD a = s & 0xff;

        if (m_dwFlags & D3DTEXTR_TRANSPARENTBLACK)
        {
            if (r == 0 && g == 0 && b == 0) a = 0;
        }
        if (m_dwFlags & D3DTEXTR_TRANSPARENTWHITE)
        {
            if (r == 0xff && g == 0xff && b == 0xff) a = 0;
        }
        dst[i] = (a<<24)|(r<<16)|(g<<8)|b;
    }
    m_pddsSurface->MarkDirty();
    return S_OK;
}

//-----------------------------------------------------------------------------
// public API
//-----------------------------------------------------------------------------

LPDIRECTDRAWSURFACE7 D3DTextr_GetSurface(TCHAR* strName)
{
    TextureContainer* ptc = FindTexture(strName);
    return ptc ? ptc->m_pddsSurface : NULL;
}

HRESULT D3DTextr_Invalidate(TCHAR* strName)
{
    TextureContainer* ptc = FindTexture(strName);
    if (ptc == NULL) return E_FAIL;
    if (ptc->m_pddsSurface != NULL) ptc->m_pddsSurface->InvalidateGL();
    return S_OK;
}

HRESULT D3DTextr_Restore(TCHAR* strName, LPDIRECT3DDEVICE7 pd3dDevice)
{
    TextureContainer* ptc = FindTexture(strName);
    if (ptc == NULL) return E_FAIL;
    return ptc->Restore(pd3dDevice);
}

HRESULT D3DTextr_InvalidateAllTextures()
{
    for (TextureContainer* ptc = g_ptcTextureList; ptc; ptc = ptc->m_pNext)
    {
        if (ptc->m_pddsSurface != NULL) ptc->m_pddsSurface->InvalidateGL();
    }
    return S_OK;
}

HRESULT D3DTextr_RestoreAllTextures(LPDIRECT3DDEVICE7 pd3dDevice)
{
    for (TextureContainer* ptc = g_ptcTextureList; ptc; ptc = ptc->m_pNext)
    {
        ptc->Restore(pd3dDevice);
    }
    return S_OK;
}

HRESULT D3DTextr_CreateTextureFromFile(TCHAR* strName, DWORD dwStage, DWORD dwFlags)
{
    if (NULL == strName || strName[0] == 0) return E_FAIL;

    // return if the texture is already loaded
    if (FindTexture(strName) != NULL) return S_OK;

    TextureContainer* ptc = new TextureContainer(strName, dwStage, dwFlags);
    if (FAILED(ptc->LoadImageData()))
    {
        delete ptc;
        return E_FAIL;
    }
    return S_OK;
}

HRESULT D3DTextr_CreateEmptyTexture(TCHAR* strName, DWORD dwWidth, DWORD dwHeight,
                                    DWORD dwStage, DWORD dwFlags)
{
    if (FindTexture(strName) != NULL) return E_FAIL;

    TextureContainer* ptc = new TextureContainer(strName, dwStage, dwFlags);
    ptc->m_dwWidth  = dwWidth;
    ptc->m_dwHeight = dwHeight;
    ptc->m_dwBPP    = 32;
    ptc->m_pRGBAData = new DWORD[dwWidth*dwHeight];
    memset(ptc->m_pRGBAData, 0, dwWidth*dwHeight*sizeof(DWORD));
    ptc->m_bHasAlpha = (dwFlags & D3DTEXTR_CREATEWITHALPHA) != 0;
    return S_OK;
}

HRESULT D3DTextr_DestroyTexture(TCHAR* strName)
{
    TextureContainer* ptc = FindTexture(strName);
    if (ptc == NULL) return E_FAIL;
    delete ptc;
    return S_OK;
}

VOID D3DTextr_SetTexturePath(TCHAR* strTexturePath)
{
    if (strTexturePath == NULL) strTexturePath = (TCHAR*)"";
    lstrcpy(g_strTexturePath, strTexturePath);
}

void D3DTextr_SetDebugMode(BOOL bDebug)
{
    g_bDebugMode = bDebug;
}
