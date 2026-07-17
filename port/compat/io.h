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

// io.h — _findfirst/_findnext shim for POSIX (Android); passes through to the
// real MinGW <io.h> on desktop Windows builds.

#ifndef _PORT_IO_H_
#define _PORT_IO_H_

#if defined(_WIN32) && !defined(__ANDROID__)

#include_next <io.h>

#else

#include <stddef.h>
#include <stdint.h>

#define _A_NORMAL 0x00
#define _A_SUBDIR 0x10

struct _finddata_t
{
    unsigned attrib;
    long     time_create;
    long     time_access;
    long     time_write;
    unsigned long size;
    char     name[260];
};

#ifdef __cplusplus
extern "C" {
#endif

intptr_t _findfirst(const char* pattern, struct _finddata_t* data);
int      _findnext(intptr_t handle, struct _finddata_t* data);
int      _findclose(intptr_t handle);

#ifdef __cplusplus
}
#endif

#endif // !_WIN32

#endif // _PORT_IO_H_
