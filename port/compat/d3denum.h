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

// D3DEnum.h — shim replacing DX7 device enumeration. The SDL/GL port exposes
// exactly one "device" (the GL context); d3dapp.h only holds a pointer.

#ifndef _PORT_D3DENUM_H_
#define _PORT_D3DENUM_H_

#include <windows.h>
#include <ddraw.h>
#include <d3d.h>

struct D3DEnum_DeviceInfo
{
    char strDesc[40];
    BOOL bHardware;
    BOOL bWindowed;
    GUID guidDevice;
};

#endif // _PORT_D3DENUM_H_
