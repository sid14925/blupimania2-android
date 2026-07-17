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

// dsound.h — DirectSound type shim. The real audio engine is a custom SDL2
// mixer (port/sdl/sdlsound.cpp). sound.h keeps its DirectSound-typed members;
// here those types map onto the port's own buffer object.

#ifndef _PORT_DSOUND_H_
#define _PORT_DSOUND_H_

#include <windows.h>

// A loaded sound: PCM data decoded to the mixer's native format
// (mono/stereo float32 at the device rate is handled inside the mixer).
struct PortSoundBuffer
{
    short*   samples;       // interleaved 16-bit PCM
    int      sampleCount;   // per channel
    int      channels;      // 1 or 2
    int      sampleRate;    // source rate (Hz)
    // playback state (owned by the mixer)
    double   cursor;        // fractional read position in frames
    BOOL     bPlaying;
    BOOL     bLoop;
    float    volume;        // 0..1
    float    pan;           // -1..+1
    float    pitch;         // frequency multiplier
};

typedef PortSoundBuffer* LPDIRECTSOUNDBUFFER;
typedef PortSoundBuffer* LPDIRECTSOUND3DBUFFER;   // same object; 3D handled by mixer
typedef void*            LPDIRECTSOUND;
typedef void*            LPDIRECTSOUND3DLISTENER;

#define DSBSTATUS_PLAYING 0x00000001

#endif // _PORT_DSOUND_H_
