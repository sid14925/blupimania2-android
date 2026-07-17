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

// dinput.h — DirectInput type shim; joystick handled by SDL2
// (port/sdl/sdljoystick.cpp implements the functions in joystick.h).

#ifndef _PORT_DINPUT_H_
#define _PORT_DINPUT_H_

#include <windows.h>

typedef struct DIJOYSTATE
{
    LONG lX;                // -1000..+1000 in the port
    LONG lY;
    LONG lZ;
    LONG lRx;
    LONG lRy;
    LONG lRz;
    LONG rglSlider[2];
    DWORD rgdwPOV[4];
    BYTE rgbButtons[32];    // 0x80 = pressed
} DIJOYSTATE, *LPDIJOYSTATE;

#endif // _PORT_DINPUT_H_
