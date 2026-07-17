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

// direct.h — passes through to MinGW on desktop; provides _mkdir/_rmdir/
// _chdir on POSIX (Android). Note: windows.h also macro-renames _mkdir to
// the path-fixing Port_mkdir for game code.

#ifndef _PORT_DIRECT_H_
#define _PORT_DIRECT_H_

#if defined(_WIN32) && !defined(__ANDROID__)

// don't let the _mkdir->Port_mkdir macro rename MinGW's dllimport declaration
#pragma push_macro("_mkdir")
#undef _mkdir
#include_next <direct.h>
#pragma pop_macro("_mkdir")

#else

#include <sys/stat.h>
#include <unistd.h>

#ifndef PORT_NO_STDIO_REDIRECT
// game code goes through the Port_* path fixers declared in windows.h
#else
static inline int _mkdir(const char* p) { return mkdir(p, 0755); }
#endif
static inline int _rmdir(const char* p) { return rmdir(p); }
static inline int _chdir(const char* p) { return chdir(p); }

#endif // !_WIN32

#endif // _PORT_DIRECT_H_
