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

// d3dutil.cpp — port implementation of the DX SDK helper functions declared
// in src/d3dutil.h (replaces the original DDraw-dependent version).

#define D3D_OVERLOADS
#define STRICT
#include <windows.h>
#include <math.h>
#include <stdio.h>
#include "D3DUtil.h"
#include "D3DMath.h"

const TCHAR* D3DUtil_GetDXSDKMediaPath()
{
    return "";
}

VOID D3DUtil_InitDeviceDesc(D3DDEVICEDESC7& ddDevDesc)
{
    memset(&ddDevDesc, 0, sizeof(D3DDEVICEDESC7));
}

VOID D3DUtil_InitSurfaceDesc(DDSURFACEDESC2& ddsd, DWORD dwFlags, DWORD dwCaps)
{
    memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags = dwFlags;
    ddsd.ddsCaps.dwCaps = dwCaps;
    ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
}

VOID D3DUtil_InitMaterial(D3DMATERIAL7& mtrl, FLOAT r, FLOAT g, FLOAT b, FLOAT a)
{
    memset(&mtrl, 0, sizeof(D3DMATERIAL7));
    mtrl.dcvDiffuse.r = mtrl.dcvAmbient.r = r;
    mtrl.dcvDiffuse.g = mtrl.dcvAmbient.g = g;
    mtrl.dcvDiffuse.b = mtrl.dcvAmbient.b = b;
    mtrl.dcvDiffuse.a = mtrl.dcvAmbient.a = a;
}

VOID D3DUtil_InitLight(D3DLIGHT7& light, D3DLIGHTTYPE ltType,
                       FLOAT x, FLOAT y, FLOAT z)
{
    memset(&light, 0, sizeof(D3DLIGHT7));
    light.dltType        = ltType;
    light.dcvDiffuse.r   = 1.0f;
    light.dcvDiffuse.g   = 1.0f;
    light.dcvDiffuse.b   = 1.0f;
    light.dcvSpecular    = light.dcvDiffuse;
    light.dvPosition.x   = light.dvDirection.x = x;
    light.dvPosition.y   = light.dvDirection.y = y;
    light.dvPosition.z   = light.dvDirection.z = z;
    light.dvAttenuation0 = 1.0f;
    light.dvRange        = D3DLIGHT_RANGE_MAX;
}

HRESULT D3DUtil_SetViewMatrix(D3DMATRIX& mat, const D3DVECTOR& vFrom,
                              const D3DVECTOR& vAt, const D3DVECTOR& vWorldUp)
{
    D3DVECTOR vView = vAt - vFrom;
    FLOAT fLength = Magnitude(vView);
    if (fLength < 1e-6f) return E_INVALIDARG;
    vView /= fLength;

    D3DVECTOR vUp = vWorldUp - vView * DotProduct(vWorldUp, vView);
    fLength = Magnitude(vUp);
    if (fLength < 1e-6f)
    {
        vUp = D3DVECTOR(0.0f, 1.0f, 0.0f) - vView * vView.y;
        fLength = Magnitude(vUp);
        if (fLength < 1e-6f)
        {
            vUp = D3DVECTOR(0.0f, 0.0f, 1.0f) - vView * vView.z;
            fLength = Magnitude(vUp);
            if (fLength < 1e-6f) return E_INVALIDARG;
        }
    }
    vUp /= fLength;

    D3DVECTOR vRight = CrossProduct(vUp, vView);

    D3DUtil_SetIdentityMatrix(mat);
    mat._11 = vRight.x;  mat._12 = vUp.x;  mat._13 = vView.x;
    mat._21 = vRight.y;  mat._22 = vUp.y;  mat._23 = vView.y;
    mat._31 = vRight.z;  mat._32 = vUp.z;  mat._33 = vView.z;
    mat._41 = -DotProduct(vFrom, vRight);
    mat._42 = -DotProduct(vFrom, vUp);
    mat._43 = -DotProduct(vFrom, vView);
    return S_OK;
}

HRESULT D3DUtil_SetProjectionMatrix(D3DMATRIX& mat, FLOAT fFOV, FLOAT fAspect,
                                    FLOAT fNearPlane, FLOAT fFarPlane)
{
    if (fabsf(fFarPlane - fNearPlane) < 0.01f) return E_INVALIDARG;
    if (fabsf(sinf(fFOV/2)) < 0.01f) return E_INVALIDARG;

    FLOAT w = fAspect * (cosf(fFOV/2)/sinf(fFOV/2));
    FLOAT h =           (cosf(fFOV/2)/sinf(fFOV/2));
    FLOAT Q = fFarPlane / (fFarPlane - fNearPlane);

    memset(&mat, 0, sizeof(D3DMATRIX));
    mat._11 = w;
    mat._22 = h;
    mat._33 = Q;
    mat._34 = 1.0f;
    mat._43 = -Q*fNearPlane;
    return S_OK;
}

VOID D3DUtil_SetRotateXMatrix(D3DMATRIX& mat, FLOAT fRads)
{
    D3DUtil_SetIdentityMatrix(mat);
    mat._22 =  cosf(fRads);
    mat._23 =  sinf(fRads);
    mat._32 = -sinf(fRads);
    mat._33 =  cosf(fRads);
}

VOID D3DUtil_SetRotateYMatrix(D3DMATRIX& mat, FLOAT fRads)
{
    D3DUtil_SetIdentityMatrix(mat);
    mat._11 =  cosf(fRads);
    mat._13 = -sinf(fRads);
    mat._31 =  sinf(fRads);
    mat._33 =  cosf(fRads);
}

VOID D3DUtil_SetRotateZMatrix(D3DMATRIX& mat, FLOAT fRads)
{
    D3DUtil_SetIdentityMatrix(mat);
    mat._11 =  cosf(fRads);
    mat._12 =  sinf(fRads);
    mat._21 = -sinf(fRads);
    mat._22 =  cosf(fRads);
}

VOID D3DUtil_SetRotationMatrix(D3DMATRIX& mat, D3DVECTOR& vDir, FLOAT fRads)
{
    FLOAT fCos = cosf(fRads);
    FLOAT fSin = sinf(fRads);
    D3DVECTOR v = Normalize(vDir);

    D3DUtil_SetIdentityMatrix(mat);
    mat._11 = (v.x * v.x) * (1.0f - fCos) + fCos;
    mat._12 = (v.x * v.y) * (1.0f - fCos) + (v.z * fSin);
    mat._13 = (v.x * v.z) * (1.0f - fCos) - (v.y * fSin);
    mat._21 = (v.y * v.x) * (1.0f - fCos) - (v.z * fSin);
    mat._22 = (v.y * v.y) * (1.0f - fCos) + fCos;
    mat._23 = (v.y * v.z) * (1.0f - fCos) + (v.x * fSin);
    mat._31 = (v.z * v.x) * (1.0f - fCos) + (v.y * fSin);
    mat._32 = (v.z * v.y) * (1.0f - fCos) - (v.x * fSin);
    mat._33 = (v.z * v.z) * (1.0f - fCos) + fCos;
}

HRESULT _DbgOut(TCHAR* strFile, DWORD dwLine, HRESULT hr, TCHAR* strMsg)
{
    printf("%s(%u): %s (hr=%08x)\n", strFile, (unsigned)dwLine,
           strMsg != NULL ? strMsg : "", (unsigned)hr);
    return hr;
}
