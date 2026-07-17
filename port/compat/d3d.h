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

// d3d.h — Direct3D 7 compatibility shim for the BlupiMania 2 SDL/OpenGL port.
// Provides the D3D7 types, enums and the IDirect3DDevice7 interface used by
// the game code, implemented over OpenGL ES 2.0 (see port/gl/gldevice.cpp).

#ifndef _PORT_D3D_H_
#define _PORT_D3D_H_

#include <windows.h>
#include <math.h>
#include <ddraw.h>

typedef float D3DVALUE;
typedef D3DVALUE* LPD3DVALUE;
typedef DWORD D3DCOLOR;
typedef D3DCOLOR* LPD3DCOLOR;

// ------------------------------------------------------------------ vectors

struct D3DVECTOR
{
    union { D3DVALUE x; D3DVALUE dvX; };
    union { D3DVALUE y; D3DVALUE dvY; };
    union { D3DVALUE z; D3DVALUE dvZ; };

    D3DVECTOR() { }
    D3DVECTOR(D3DVALUE f) { x = y = z = f; }
    D3DVECTOR(D3DVALUE _x, D3DVALUE _y, D3DVALUE _z) { x = _x; y = _y; z = _z; }

    // assignment operators
    D3DVECTOR& operator += (const D3DVECTOR& v) { x += v.x; y += v.y; z += v.z; return *this; }
    D3DVECTOR& operator -= (const D3DVECTOR& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    D3DVECTOR& operator *= (const D3DVECTOR& v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
    D3DVECTOR& operator /= (const D3DVECTOR& v) { x /= v.x; y /= v.y; z /= v.z; return *this; }
    D3DVECTOR& operator *= (D3DVALUE s) { x *= s; y *= s; z *= s; return *this; }
    D3DVECTOR& operator /= (D3DVALUE s) { x /= s; y /= s; z /= s; return *this; }

    // unary operators
    friend D3DVECTOR operator + (const D3DVECTOR& v) { return v; }
    friend D3DVECTOR operator - (const D3DVECTOR& v) { return D3DVECTOR(-v.x, -v.y, -v.z); }

    // binary operators
    friend D3DVECTOR operator + (const D3DVECTOR& a, const D3DVECTOR& b) { return D3DVECTOR(a.x+b.x, a.y+b.y, a.z+b.z); }
    friend D3DVECTOR operator - (const D3DVECTOR& a, const D3DVECTOR& b) { return D3DVECTOR(a.x-b.x, a.y-b.y, a.z-b.z); }
    friend D3DVECTOR operator * (const D3DVECTOR& a, const D3DVECTOR& b) { return D3DVECTOR(a.x*b.x, a.y*b.y, a.z*b.z); }
    friend D3DVECTOR operator / (const D3DVECTOR& a, const D3DVECTOR& b) { return D3DVECTOR(a.x/b.x, a.y/b.y, a.z/b.z); }
    friend D3DVECTOR operator * (const D3DVECTOR& v, D3DVALUE s) { return D3DVECTOR(v.x*s, v.y*s, v.z*s); }
    friend D3DVECTOR operator * (D3DVALUE s, const D3DVECTOR& v) { return D3DVECTOR(v.x*s, v.y*s, v.z*s); }
    friend D3DVECTOR operator / (const D3DVECTOR& v, D3DVALUE s) { return D3DVECTOR(v.x/s, v.y/s, v.z/s); }

    friend BOOL operator == (const D3DVECTOR& a, const D3DVECTOR& b) { return a.x==b.x && a.y==b.y && a.z==b.z; }
    friend BOOL operator != (const D3DVECTOR& a, const D3DVECTOR& b) { return a.x!=b.x || a.y!=b.y || a.z!=b.z; }
};
typedef D3DVECTOR* LPD3DVECTOR;

inline D3DVALUE SquareMagnitude(const D3DVECTOR& v) { return v.x*v.x + v.y*v.y + v.z*v.z; }
inline D3DVALUE Magnitude(const D3DVECTOR& v) { return (D3DVALUE)sqrtf(SquareMagnitude(v)); }
inline D3DVECTOR Normalize(const D3DVECTOR& v) { return v / Magnitude(v); }
inline D3DVALUE Min(const D3DVECTOR& v) { D3DVALUE r = v.x; if (v.y < r) r = v.y; if (v.z < r) r = v.z; return r; }
inline D3DVALUE Max(const D3DVECTOR& v) { D3DVALUE r = v.x; if (v.y > r) r = v.y; if (v.z > r) r = v.z; return r; }
inline D3DVALUE DotProduct(const D3DVECTOR& a, const D3DVECTOR& b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline D3DVECTOR CrossProduct(const D3DVECTOR& a, const D3DVECTOR& b)
{
    return D3DVECTOR(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}

// ------------------------------------------------------------------- matrix

struct D3DMATRIX
{
    union { struct {
        D3DVALUE _11, _12, _13, _14;
        D3DVALUE _21, _22, _23, _24;
        D3DVALUE _31, _32, _33, _34;
        D3DVALUE _41, _42, _43, _44;
    }; D3DVALUE m[4][4]; };

    D3DMATRIX() { }
    D3DVALUE& operator () (int row, int col) { return m[row][col]; }
    const D3DVALUE& operator () (int row, int col) const { return m[row][col]; }

    friend D3DMATRIX operator * (const D3DMATRIX& a, const D3DMATRIX& b)
    {
        D3DMATRIX q;
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                q.m[i][j] = a.m[i][0]*b.m[0][j] + a.m[i][1]*b.m[1][j] +
                            a.m[i][2]*b.m[2][j] + a.m[i][3]*b.m[3][j];
        return q;
    }
};
typedef D3DMATRIX* LPD3DMATRIX;

// -------------------------------------------------------------------- color

struct D3DCOLORVALUE
{
    union { D3DVALUE r; D3DVALUE dvR; };
    union { D3DVALUE g; D3DVALUE dvG; };
    union { D3DVALUE b; D3DVALUE dvB; };
    union { D3DVALUE a; D3DVALUE dvA; };
};
typedef D3DCOLORVALUE* LPD3DCOLORVALUE;

#define RGB_MAKE(r,g,b)     ((D3DCOLOR)(((r)<<16)|((g)<<8)|(b)))
#define RGBA_MAKE(r,g,b,a)  ((D3DCOLOR)(((a)<<24)|((r)<<16)|((g)<<8)|(b)))
#define D3DRGB(r,g,b) \
    (0xff000000L | (((LONG)((r)*255)) << 16) | (((LONG)((g)*255)) << 8) | (LONG)((b)*255))
#define D3DRGBA(r,g,b,a) \
    ((((LONG)((a)*255)) << 24) | (((LONG)((r)*255)) << 16) | (((LONG)((g)*255)) << 8) | (LONG)((b)*255))
#define RGB_GETRED(rgb)     (((rgb) >> 16) & 0xff)
#define RGB_GETGREEN(rgb)   (((rgb) >> 8) & 0xff)
#define RGB_GETBLUE(rgb)    ((rgb) & 0xff)
#define RGBA_GETALPHA(rgb)  ((rgb) >> 24)
#define RGBA_GETRED(rgb)    (((rgb) >> 16) & 0xff)
#define RGBA_GETGREEN(rgb)  (((rgb) >> 8) & 0xff)
#define RGBA_GETBLUE(rgb)   ((rgb) & 0xff)

// ------------------------------------------------------------------ structs

typedef struct _D3DRECT
{
    LONG x1, y1, x2, y2;
} D3DRECT, *LPD3DRECT;

typedef struct _D3DMATERIAL7
{
    union { D3DCOLORVALUE diffuse;  D3DCOLORVALUE dcvDiffuse; };
    union { D3DCOLORVALUE ambient;  D3DCOLORVALUE dcvAmbient; };
    union { D3DCOLORVALUE specular; D3DCOLORVALUE dcvSpecular; };
    union { D3DCOLORVALUE emissive; D3DCOLORVALUE dcvEmissive; };
    union { D3DVALUE      power;    D3DVALUE      dvPower; };
} D3DMATERIAL7, *LPD3DMATERIAL7;

typedef enum _D3DLIGHTTYPE
{
    D3DLIGHT_POINT       = 1,
    D3DLIGHT_SPOT        = 2,
    D3DLIGHT_DIRECTIONAL = 3,
    D3DLIGHT_FORCE_DWORD = 0x7fffffff
} D3DLIGHTTYPE;

typedef struct _D3DLIGHT7
{
    D3DLIGHTTYPE    dltType;
    D3DCOLORVALUE   dcvDiffuse;
    D3DCOLORVALUE   dcvSpecular;
    D3DCOLORVALUE   dcvAmbient;
    D3DVECTOR       dvPosition;
    D3DVECTOR       dvDirection;
    D3DVALUE        dvRange;
    D3DVALUE        dvFalloff;
    D3DVALUE        dvAttenuation0;
    D3DVALUE        dvAttenuation1;
    D3DVALUE        dvAttenuation2;
    D3DVALUE        dvTheta;
    D3DVALUE        dvPhi;
} D3DLIGHT7, *LPD3DLIGHT7;

#define D3DLIGHT_RANGE_MAX  ((D3DVALUE)sqrt(3.402823466e+38f))

typedef struct _D3DVIEWPORT7
{
    DWORD    dwX;
    DWORD    dwY;
    DWORD    dwWidth;
    DWORD    dwHeight;
    D3DVALUE dvMinZ;
    D3DVALUE dvMaxZ;
} D3DVIEWPORT7, *LPD3DVIEWPORT7;

// standard vertex types
typedef struct _D3DVERTEX
{
    union { D3DVALUE x;  D3DVALUE dvX; };
    union { D3DVALUE y;  D3DVALUE dvY; };
    union { D3DVALUE z;  D3DVALUE dvZ; };
    union { D3DVALUE nx; D3DVALUE dvNX; };
    union { D3DVALUE ny; D3DVALUE dvNY; };
    union { D3DVALUE nz; D3DVALUE dvNZ; };
    union { D3DVALUE tu; D3DVALUE dvTU; };
    union { D3DVALUE tv; D3DVALUE dvTV; };

    _D3DVERTEX() { }
    _D3DVERTEX(const D3DVECTOR& v, const D3DVECTOR& n, float _tu, float _tv)
    {
        x = v.x; y = v.y; z = v.z;
        nx = n.x; ny = n.y; nz = n.z;
        tu = _tu; tv = _tv;
    }
} D3DVERTEX, *LPD3DVERTEX;

typedef struct _D3DLVERTEX
{
    union { D3DVALUE x; D3DVALUE dvX; };
    union { D3DVALUE y; D3DVALUE dvY; };
    union { D3DVALUE z; D3DVALUE dvZ; };
    DWORD    dwReserved;
    union { D3DCOLOR color;    D3DCOLOR dcColor; };
    union { D3DCOLOR specular; D3DCOLOR dcSpecular; };
    union { D3DVALUE tu; D3DVALUE dvTU; };
    union { D3DVALUE tv; D3DVALUE dvTV; };

    _D3DLVERTEX() { }
    _D3DLVERTEX(const D3DVECTOR& v, D3DCOLOR col, D3DCOLOR spec, float _tu, float _tv)
    {
        x = v.x; y = v.y; z = v.z;
        dwReserved = 0;
        color = col; specular = spec;
        tu = _tu; tv = _tv;
    }
} D3DLVERTEX, *LPD3DLVERTEX;

typedef struct _D3DTLVERTEX
{
    union { D3DVALUE sx; D3DVALUE dvSX; };
    union { D3DVALUE sy; D3DVALUE dvSY; };
    union { D3DVALUE sz; D3DVALUE dvSZ; };
    union { D3DVALUE rhw; D3DVALUE dvRHW; };
    union { D3DCOLOR color;    D3DCOLOR dcColor; };
    union { D3DCOLOR specular; D3DCOLOR dcSpecular; };
    union { D3DVALUE tu; D3DVALUE dvTU; };
    union { D3DVALUE tv; D3DVALUE dvTV; };
} D3DTLVERTEX, *LPD3DTLVERTEX;

// -------------------------------------------------------------------- enums

typedef enum _D3DPRIMITIVETYPE
{
    D3DPT_POINTLIST     = 1,
    D3DPT_LINELIST      = 2,
    D3DPT_LINESTRIP     = 3,
    D3DPT_TRIANGLELIST  = 4,
    D3DPT_TRIANGLESTRIP = 5,
    D3DPT_TRIANGLEFAN   = 6,
    D3DPT_FORCE_DWORD   = 0x7fffffff
} D3DPRIMITIVETYPE;

typedef enum _D3DTRANSFORMSTATETYPE
{
    D3DTRANSFORMSTATE_WORLD      = 1,
    D3DTRANSFORMSTATE_VIEW       = 2,
    D3DTRANSFORMSTATE_PROJECTION = 3,
    D3DTRANSFORMSTATE_WORLD1     = 4,
    D3DTRANSFORMSTATE_TEXTURE0   = 16,
    D3DTRANSFORMSTATE_TEXTURE1   = 17,
    D3DTRANSFORMSTATE_FORCE_DWORD = 0x7fffffff
} D3DTRANSFORMSTATETYPE;

typedef enum _D3DFILLMODE
{
    D3DFILL_POINT     = 1,
    D3DFILL_WIREFRAME = 2,
    D3DFILL_SOLID     = 3,
    D3DFILL_FORCE_DWORD = 0x7fffffff
} D3DFILLMODE;

typedef enum _D3DSHADEMODE
{
    D3DSHADE_FLAT    = 1,
    D3DSHADE_GOURAUD = 2,
    D3DSHADE_PHONG   = 3,
    D3DSHADE_FORCE_DWORD = 0x7fffffff
} D3DSHADEMODE;

typedef enum _D3DCULL
{
    D3DCULL_NONE = 1,
    D3DCULL_CW   = 2,
    D3DCULL_CCW  = 3,
    D3DCULL_FORCE_DWORD = 0x7fffffff
} D3DCULL;

typedef enum _D3DCMPFUNC
{
    D3DCMP_NEVER        = 1,
    D3DCMP_LESS         = 2,
    D3DCMP_EQUAL        = 3,
    D3DCMP_LESSEQUAL    = 4,
    D3DCMP_GREATER      = 5,
    D3DCMP_NOTEQUAL     = 6,
    D3DCMP_GREATEREQUAL = 7,
    D3DCMP_ALWAYS       = 8,
    D3DCMP_FORCE_DWORD  = 0x7fffffff
} D3DCMPFUNC;

typedef enum _D3DBLEND
{
    D3DBLEND_ZERO            = 1,
    D3DBLEND_ONE             = 2,
    D3DBLEND_SRCCOLOR        = 3,
    D3DBLEND_INVSRCCOLOR     = 4,
    D3DBLEND_SRCALPHA        = 5,
    D3DBLEND_INVSRCALPHA     = 6,
    D3DBLEND_DESTALPHA       = 7,
    D3DBLEND_INVDESTALPHA    = 8,
    D3DBLEND_DESTCOLOR       = 9,
    D3DBLEND_INVDESTCOLOR    = 10,
    D3DBLEND_SRCALPHASAT     = 11,
    D3DBLEND_BOTHSRCALPHA    = 12,
    D3DBLEND_BOTHINVSRCALPHA = 13,
    D3DBLEND_FORCE_DWORD     = 0x7fffffff
} D3DBLEND;

typedef enum _D3DFOGMODE
{
    D3DFOG_NONE   = 0,
    D3DFOG_EXP    = 1,
    D3DFOG_EXP2   = 2,
    D3DFOG_LINEAR = 3,
    D3DFOG_FORCE_DWORD = 0x7fffffff
} D3DFOGMODE;

typedef enum _D3DRENDERSTATETYPE
{
    D3DRENDERSTATE_ANTIALIAS         = 2,
    D3DRENDERSTATE_TEXTUREPERSPECTIVE = 4,
    D3DRENDERSTATE_ZENABLE           = 7,
    D3DRENDERSTATE_FILLMODE          = 8,
    D3DRENDERSTATE_SHADEMODE         = 9,
    D3DRENDERSTATE_LINEPATTERN       = 10,
    D3DRENDERSTATE_ZWRITEENABLE      = 14,
    D3DRENDERSTATE_ALPHATESTENABLE   = 15,
    D3DRENDERSTATE_LASTPIXEL         = 16,
    D3DRENDERSTATE_SRCBLEND          = 19,
    D3DRENDERSTATE_DESTBLEND         = 20,
    D3DRENDERSTATE_CULLMODE          = 22,
    D3DRENDERSTATE_ZFUNC             = 23,
    D3DRENDERSTATE_ALPHAREF          = 24,
    D3DRENDERSTATE_ALPHAFUNC         = 25,
    D3DRENDERSTATE_DITHERENABLE      = 26,
    D3DRENDERSTATE_ALPHABLENDENABLE  = 27,
    D3DRENDERSTATE_FOGENABLE         = 28,
    D3DRENDERSTATE_SPECULARENABLE    = 29,
    D3DRENDERSTATE_ZVISIBLE          = 30,
    D3DRENDERSTATE_STIPPLEDALPHA     = 33,
    D3DRENDERSTATE_FOGCOLOR          = 34,
    D3DRENDERSTATE_FOGTABLEMODE      = 35,
    D3DRENDERSTATE_FOGSTART          = 36,
    D3DRENDERSTATE_FOGEND            = 37,
    D3DRENDERSTATE_FOGDENSITY        = 38,
    D3DRENDERSTATE_EDGEANTIALIAS     = 40,
    D3DRENDERSTATE_COLORKEYENABLE    = 41,
    D3DRENDERSTATE_ZBIAS             = 47,
    D3DRENDERSTATE_RANGEFOGENABLE    = 48,
    D3DRENDERSTATE_STENCILENABLE     = 52,
    D3DRENDERSTATE_STENCILFAIL       = 53,
    D3DRENDERSTATE_STENCILZFAIL      = 54,
    D3DRENDERSTATE_STENCILPASS       = 55,
    D3DRENDERSTATE_STENCILFUNC       = 56,
    D3DRENDERSTATE_STENCILREF        = 57,
    D3DRENDERSTATE_STENCILMASK       = 58,
    D3DRENDERSTATE_STENCILWRITEMASK  = 59,
    D3DRENDERSTATE_TEXTUREFACTOR     = 60,
    D3DRENDERSTATE_WRAP0             = 128,
    D3DRENDERSTATE_WRAP1             = 129,
    D3DRENDERSTATE_WRAP2             = 130,
    D3DRENDERSTATE_WRAP3             = 131,
    D3DRENDERSTATE_WRAP4             = 132,
    D3DRENDERSTATE_WRAP5             = 133,
    D3DRENDERSTATE_WRAP6             = 134,
    D3DRENDERSTATE_WRAP7             = 135,
    D3DRENDERSTATE_CLIPPING          = 136,
    D3DRENDERSTATE_LIGHTING          = 137,
    D3DRENDERSTATE_EXTENTS           = 138,
    D3DRENDERSTATE_AMBIENT           = 139,
    D3DRENDERSTATE_FOGVERTEXMODE     = 140,
    D3DRENDERSTATE_COLORVERTEX       = 141,
    D3DRENDERSTATE_LOCALVIEWER       = 142,
    D3DRENDERSTATE_NORMALIZENORMALS  = 143,
    D3DRENDERSTATE_FORCE_DWORD       = 0x7fffffff
} D3DRENDERSTATETYPE;

#define D3DWRAP_U 0x00000001L
#define D3DWRAP_V 0x00000002L

typedef enum _D3DTEXTURESTAGESTATETYPE
{
    D3DTSS_COLOROP        = 1,
    D3DTSS_COLORARG1      = 2,
    D3DTSS_COLORARG2      = 3,
    D3DTSS_ALPHAOP        = 4,
    D3DTSS_ALPHAARG1      = 5,
    D3DTSS_ALPHAARG2      = 6,
    D3DTSS_BUMPENVMAT00   = 7,
    D3DTSS_BUMPENVMAT01   = 8,
    D3DTSS_BUMPENVMAT10   = 9,
    D3DTSS_BUMPENVMAT11   = 10,
    D3DTSS_TEXCOORDINDEX  = 11,
    D3DTSS_ADDRESS        = 12,
    D3DTSS_ADDRESSU       = 13,
    D3DTSS_ADDRESSV       = 14,
    D3DTSS_BORDERCOLOR    = 15,
    D3DTSS_MAGFILTER      = 16,
    D3DTSS_MINFILTER      = 17,
    D3DTSS_MIPFILTER      = 18,
    D3DTSS_MIPMAPLODBIAS  = 19,
    D3DTSS_MAXMIPLEVEL    = 20,
    D3DTSS_MAXANISOTROPY  = 21,
    D3DTSS_BUMPENVLSCALE  = 22,
    D3DTSS_BUMPENVLOFFSET = 23,
    D3DTSS_TEXTURETRANSFORMFLAGS = 24,
    D3DTSS_FORCE_DWORD    = 0x7fffffff
} D3DTEXTURESTAGESTATETYPE;

typedef enum _D3DTEXTUREOP
{
    D3DTOP_DISABLE   = 1,
    D3DTOP_SELECTARG1 = 2,
    D3DTOP_SELECTARG2 = 3,
    D3DTOP_MODULATE  = 4,
    D3DTOP_MODULATE2X = 5,
    D3DTOP_MODULATE4X = 6,
    D3DTOP_ADD       = 7,
    D3DTOP_ADDSIGNED = 8,
    D3DTOP_SUBTRACT  = 10,
    D3DTOP_BLENDDIFFUSEALPHA = 12,
    D3DTOP_BLENDTEXTUREALPHA = 13,
    D3DTOP_BLENDFACTORALPHA  = 14,
    D3DTOP_FORCE_DWORD = 0x7fffffff
} D3DTEXTUREOP;

#define D3DTA_SELECTMASK     0x0000000f
#define D3DTA_DIFFUSE        0x00000000
#define D3DTA_CURRENT        0x00000001
#define D3DTA_TEXTURE        0x00000002
#define D3DTA_TFACTOR        0x00000003
#define D3DTA_COMPLEMENT     0x00000010
#define D3DTA_ALPHAREPLICATE 0x00000020

typedef enum _D3DTEXTUREMAGFILTER
{
    D3DTFG_POINT  = 1,
    D3DTFG_LINEAR = 2,
    D3DTFG_FORCE_DWORD = 0x7fffffff
} D3DTEXTUREMAGFILTER;

typedef enum _D3DTEXTUREMINFILTER
{
    D3DTFN_POINT  = 1,
    D3DTFN_LINEAR = 2,
    D3DTFN_FORCE_DWORD = 0x7fffffff
} D3DTEXTUREMINFILTER;

typedef enum _D3DTEXTUREMIPFILTER
{
    D3DTFP_NONE   = 1,
    D3DTFP_POINT  = 2,
    D3DTFP_LINEAR = 3,
    D3DTFP_FORCE_DWORD = 0x7fffffff
} D3DTEXTUREMIPFILTER;

typedef enum _D3DTEXTUREADDRESS
{
    D3DTADDRESS_WRAP   = 1,
    D3DTADDRESS_MIRROR = 2,
    D3DTADDRESS_CLAMP  = 3,
    D3DTADDRESS_BORDER = 4,
    D3DTADDRESS_FORCE_DWORD = 0x7fffffff
} D3DTEXTUREADDRESS;

// flexible vertex format bits
#define D3DFVF_RESERVED0     0x001
#define D3DFVF_XYZ           0x002
#define D3DFVF_XYZRHW        0x004
#define D3DFVF_NORMAL        0x010
#define D3DFVF_RESERVED1     0x020
#define D3DFVF_DIFFUSE       0x040
#define D3DFVF_SPECULAR      0x080
#define D3DFVF_TEXCOUNT_MASK 0xf00
#define D3DFVF_TEXCOUNT_SHIFT 8
#define D3DFVF_TEX0          0x000
#define D3DFVF_TEX1          0x100
#define D3DFVF_TEX2          0x200
#define D3DFVF_TEX3          0x300

#define D3DFVF_VERTEX   (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)
#define D3DFVF_LVERTEX  (D3DFVF_XYZ | D3DFVF_RESERVED1 | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1)
#define D3DFVF_TLVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1)

// Clear flags
#define D3DCLEAR_TARGET  0x00000001
#define D3DCLEAR_ZBUFFER 0x00000002
#define D3DCLEAR_STENCIL 0x00000004

// DrawPrimitive flags
#define D3DDP_WAIT 0x00000001

// ComputeSphereVisibility results
#define D3DVIS_INSIDE_FRUSTUM  0
#define D3DVIS_INTERSECT_FRUSTUM 1
#define D3DVIS_OUTSIDE_FRUSTUM 2

// ComputeSphereVisibility status bits (subset): a CLIPINTERSECTION* bit is set
// when the sphere lies entirely outside that clip plane
#define D3DSTATUS_CLIPUNIONLEFT         0x00000010
#define D3DSTATUS_CLIPUNIONRIGHT        0x00000020
#define D3DSTATUS_CLIPUNIONTOP          0x00000040
#define D3DSTATUS_CLIPUNIONBOTTOM       0x00000080
#define D3DSTATUS_CLIPUNIONFRONT        0x00000100
#define D3DSTATUS_CLIPUNIONBACK         0x00000200
#define D3DSTATUS_CLIPUNIONALL          0x000003F0
#define D3DSTATUS_CLIPINTERSECTIONLEFT  0x00001000
#define D3DSTATUS_CLIPINTERSECTIONRIGHT 0x00002000
#define D3DSTATUS_CLIPINTERSECTIONTOP   0x00004000
#define D3DSTATUS_CLIPINTERSECTIONBOTTOM 0x00008000
#define D3DSTATUS_CLIPINTERSECTIONFRONT 0x00010000
#define D3DSTATUS_CLIPINTERSECTIONBACK  0x00020000
#define D3DSTATUS_CLIPINTERSECTIONALL   0x0003F000

// Light state
#define D3DLIGHT_RANGE(x) (x)

// caps structs — only what the engine actually inspects
#define D3DPTBLENDCAPS_ADD          0x00000004
#define D3DPRASTERCAPS_ZBUFFERLESSHSR 0x00008000
#define D3DPTEXTURECAPS_POW2        0x00000002
typedef struct _D3DPRIMCAPS
{
    DWORD dwSize;
    DWORD dwMiscCaps;
    DWORD dwRasterCaps;
    DWORD dwZCmpCaps;
    DWORD dwSrcBlendCaps;
    DWORD dwDestBlendCaps;
    DWORD dwAlphaCmpCaps;
    DWORD dwShadeCaps;
    DWORD dwTextureCaps;
    DWORD dwTextureFilterCaps;
    DWORD dwTextureBlendCaps;
    DWORD dwTextureAddressCaps;
} D3DPRIMCAPS;
typedef struct _D3DDEVICEDESC7
{
    DWORD dwDevCaps;
    D3DPRIMCAPS dpcLineCaps;
    D3DPRIMCAPS dpcTriCaps;
    DWORD dwMaxTextureWidth;
    DWORD dwMaxTextureHeight;
    DWORD wMaxSimultaneousTextures;
    DWORD dwMaxActiveLights;
    GUID  deviceGUID;
} D3DDEVICEDESC7, *LPD3DDEVICEDESC7;

// ------------------------------------------------------------------ device

class IDirectDrawSurface7;      // defined in ddraw.h shim
typedef IDirectDrawSurface7* LPDIRECTDRAWSURFACE7;

// IDirect3DDevice7 implemented over OpenGL ES 2.0.
class IDirect3DDevice7
{
public:
    IDirect3DDevice7();
    ~IDirect3DDevice7();

    // called by the SDL platform layer
    HRESULT InitDevice(int width, int height);
    void    Resize(int width, int height);
    void    Present();

    // D3D7 interface used by the game
    HRESULT BeginScene();
    HRESULT EndScene();
    HRESULT Clear(DWORD dwCount, LPD3DRECT lpRects, DWORD dwFlags,
                  D3DCOLOR dwColor, D3DVALUE dvZ, DWORD dwStencil);
    HRESULT SetTransform(D3DTRANSFORMSTATETYPE dtstTransformStateType,
                         LPD3DMATRIX lpD3DMatrix);
    HRESULT GetTransform(D3DTRANSFORMSTATETYPE dtstTransformStateType,
                         LPD3DMATRIX lpD3DMatrix);
    HRESULT SetViewport(LPD3DVIEWPORT7 lpViewport);
    HRESULT GetViewport(LPD3DVIEWPORT7 lpViewport);
    HRESULT SetMaterial(LPD3DMATERIAL7 lpMaterial);
    HRESULT GetMaterial(LPD3DMATERIAL7 lpMaterial);
    HRESULT SetLight(DWORD dwLightIndex, LPD3DLIGHT7 lpLight);
    HRESULT GetLight(DWORD dwLightIndex, LPD3DLIGHT7 lpLight);
    HRESULT LightEnable(DWORD dwLightIndex, BOOL bEnable);
    HRESULT GetLightEnable(DWORD dwLightIndex, BOOL* pbEnable);
    HRESULT SetRenderState(D3DRENDERSTATETYPE dwRenderStateType, DWORD dwRenderState);
    HRESULT GetRenderState(D3DRENDERSTATETYPE dwRenderStateType, LPDWORD lpdwRenderState);
    HRESULT SetTexture(DWORD dwStage, LPDIRECTDRAWSURFACE7 lpTexture);
    HRESULT SetTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE dwState, DWORD dwValue);
    HRESULT DrawPrimitive(D3DPRIMITIVETYPE dptPrimitiveType, DWORD dwVertexTypeDesc,
                          LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags);
    HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE dptPrimitiveType, DWORD dwVertexTypeDesc,
                                 LPVOID lpvVertices, DWORD dwVertexCount,
                                 LPWORD lpwIndices, DWORD dwIndexCount, DWORD dwFlags);
    HRESULT ComputeSphereVisibility(LPD3DVECTOR lpCenters, LPD3DVALUE lpRadii,
                                    DWORD dwNumSpheres, DWORD dwFlags, LPDWORD lpdwReturnValues);
    HRESULT SetRenderTarget(LPDIRECTDRAWSURFACE7 lpNewRenderTarget, DWORD dwFlags);
    HRESULT GetCaps(LPD3DDEVICEDESC7 lpD3DDevDesc);
    HRESULT ValidateDevice(LPDWORD lpdwPasses);
    ULONG   Release() { return 0; }

private:
    struct Impl;
    Impl* m;
    void  ApplyState(DWORD fvf);
    void  UpdateMatrices();
};

typedef IDirect3DDevice7* LPDIRECT3DDEVICE7;

// opaque legacy interfaces kept as forward decls so headers compile
class IDirectDraw7;   typedef IDirectDraw7*  LPDIRECTDRAW7;
class IDirect3D7;     typedef IDirect3D7*    LPDIRECT3D7;

#endif // _PORT_D3D_H_
