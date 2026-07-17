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

// shlobj.h — shell folder shim. CSIDL_PERSONAL ("My Documents") maps to the
// port's writable data directory (SDL pref path on Android, cwd on desktop).

#ifndef _PORT_SHLOBJ_H_
#define _PORT_SHLOBJ_H_

#include <windows.h>

typedef void* LPITEMIDLIST;
#define CSIDL_PERSONAL 0x0005

#ifdef __cplusplus
extern "C" {
#endif

HRESULT SHGetSpecialFolderLocation(HWND hwnd, int csidl, LPITEMIDLIST* ppidl);
BOOL    SHGetPathFromIDList(LPITEMIDLIST pidl, char* pszPath);

#ifdef __cplusplus
}
#endif

#endif // _PORT_SHLOBJ_H_
