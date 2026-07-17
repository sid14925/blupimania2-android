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

// windows.h — Win32 compatibility shim for the BlupiMania 2 SDL/OpenGL port.
// This header replaces the real <windows.h> on all platforms (including the
// Windows desktop test build, which uses SDL2 instead of Win32 directly).

#ifndef _PORT_WINDOWS_H_
#define _PORT_WINDOWS_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

// ---------------------------------------------------------------- base types

typedef int32_t          BOOL;
typedef uint8_t          BYTE;
typedef uint16_t         WORD;
typedef uint32_t         DWORD;
typedef int32_t          LONG;
typedef uint32_t         ULONG;
typedef unsigned int     UINT;
typedef int              INT;
typedef float            FLOAT;
typedef char             CHAR;
typedef char             TCHAR;
typedef void             VOID;
typedef int32_t          HRESULT;

typedef char*            LPSTR;
typedef char*            LPTSTR;
typedef const char*      LPCSTR;
typedef const char*      LPCTSTR;
typedef void*            LPVOID;
typedef const void*      LPCVOID;
typedef BYTE*            LPBYTE;
typedef DWORD*           LPDWORD;
typedef WORD*            LPWORD;

typedef void*            HANDLE;
typedef void*            HWND;
typedef void*            HINSTANCE;
typedef void*            HMODULE;
typedef void*            HDC;
typedef void*            HKEY;
typedef void*            HCURSOR;
typedef void*            HICON;
typedef void*            HMENU;
typedef void*            HGDIOBJ;
typedef void*            HFONT;
typedef void*            HBITMAP;
typedef void*            HRSRC;
typedef void*            HGLOBAL;
typedef void*            HACCEL;

typedef uintptr_t        WPARAM;
typedef intptr_t         LPARAM;
typedef intptr_t         LRESULT;

typedef struct tagGUID
{
    DWORD Data1;
    WORD  Data2;
    WORD  Data3;
    BYTE  Data4[8];
} GUID;
typedef GUID* LPGUID;
typedef const GUID& REFIID;
typedef const GUID& REFGUID;

typedef struct tagPOINT { LONG x; LONG y; }               POINT, *LPPOINT;
typedef struct tagRECT  { LONG left, top, right, bottom; } RECT, *LPRECT;
typedef struct tagSIZE  { LONG cx; LONG cy; }             SIZE;

// ------------------------------------------------------------------- macros

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

#ifndef NULL
#define NULL    0
#endif

#define WINAPI
#define CALLBACK
#define APIENTRY
#define FAR
#define NEAR
#define PASCAL
#define __cdecl_shim
#define STRICT_SHIM

#define MAX_PATH        260
#define _MAX_PATH       260
#define _MAX_FNAME      256
#define _MAX_EXT        256
#define _MAX_DIR        256
#define _MAX_DRIVE      3

#define S_OK            ((HRESULT)0)
#define S_FALSE         ((HRESULT)1)
#define E_FAIL          ((HRESULT)0x80004005L)
#define E_OUTOFMEMORY   ((HRESULT)0x8007000EL)
#define E_INVALIDARG    ((HRESULT)0x80070057L)
#define E_NOTIMPL       ((HRESULT)0x80004001L)

#define SUCCEEDED(hr)   ((HRESULT)(hr) >= 0)
#define FAILED(hr)      ((HRESULT)(hr) < 0)

#define LOWORD(l)       ((WORD)((uintptr_t)(l) & 0xffff))
#define HIWORD(l)       ((WORD)(((uintptr_t)(l) >> 16) & 0xffff))
#define LOBYTE(w)       ((BYTE)((uintptr_t)(w) & 0xff))
#define HIBYTE(w)       ((BYTE)(((uintptr_t)(w) >> 8) & 0xff))
#define MAKELONG(a,b)   ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#define MAKEWORD(a,b)   ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))

#define ZeroMemory(p,n)     memset((p), 0, (n))
#define FillMemory(p,n,v)   memset((p), (v), (n))
#define CopyMemory(d,s,n)   memcpy((d), (s), (n))
#define MoveMemory(d,s,n)   memmove((d), (s), (n))

// GetAsyncKeyState / GetKeyState state bit
#define KEYSTATE_DOWN 0x8000

// this codebase expects the Win32 min/max macros unconditionally
// (libstdc++ on MinGW defines NOMINMAX, so no NOMINMAX guard here)
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

// ------------------------------------------------------------ string compat

#define lstrlen(s)          ((int)strlen(s))
#define lstrcpy(d,s)        strcpy((d),(s))
#define lstrcpyn(d,s,n)     PortStrCpyN((d),(s),(n))
#define lstrcat(d,s)        strcat((d),(s))
#define lstrcmp(a,b)        strcmp((a),(b))
#define lstrcmpi(a,b)       PortStriCmp((a),(b))
#define wsprintf            sprintf
#define wvsprintf           vsprintf

#ifndef _MSC_VER
#define _stricmp            PortStriCmp
#define _strnicmp           PortStrniCmp
#define stricmp             PortStriCmp
#define strnicmp            PortStrniCmp
#define _T(x)               x
#endif

// Rename every shim that shares a name with a real Win32 export, so the
// desktop MinGW build never collides with kernel32/user32 import libraries.
#define OutputDebugString   Port_OutputDebugString
#define timeGetTime         Port_timeGetTime
#define Sleep               Port_Sleep
#define timeBeginPeriod     Port_timeBeginPeriod
#define timeEndPeriod       Port_timeEndPeriod
#define GetPrivateProfileString  Port_GetPrivateProfileString
#define GetPrivateProfileInt     Port_GetPrivateProfileInt
#define WritePrivateProfileString Port_WritePrivateProfileString
#define MessageBox          Port_MessageBox
#define ShowCursor          Port_ShowCursor
#define SetCursorPos        Port_SetCursorPos
#define GetCursorPos        Port_GetCursorPos
#define GetKeyState         Port_GetKeyState
#define GetKeyboardState    Port_GetKeyboardState
#define GetCurrentProcess   Port_GetCurrentProcess
#define GetCurrentThread    Port_GetCurrentThread
#define SetPriorityClass    Port_SetPriorityClass
#define SetThreadPriority   Port_SetThreadPriority
#define RegOpenKeyEx        Port_RegOpenKeyEx
#define RegQueryValueEx     Port_RegQueryValueEx
#define RegCloseKey         Port_RegCloseKey
#define CreateFile          Port_CreateFile
#define ReadFile            Port_ReadFile
#define WriteFile           Port_WriteFile
#define CloseHandle         Port_CloseHandle
#define LoadString          Port_LoadString
#define LoadCursor          Port_LoadCursor
#define SetCursor           Port_SetCursor
#define GlobalAlloc         Port_GlobalAlloc
#define GlobalLock          Port_GlobalLock
#define GlobalUnlock        Port_GlobalUnlock
#define GlobalFree          Port_GlobalFree
#define OpenClipboard       Port_OpenClipboard
#define EmptyClipboard      Port_EmptyClipboard
#define SetClipboardData    Port_SetClipboardData
#define GetClipboardData    Port_GetClipboardData
#define CloseClipboard      Port_CloseClipboard

#ifdef __cplusplus
extern "C" {
#endif

int   PortStriCmp(const char* a, const char* b);
int   PortStrniCmp(const char* a, const char* b, size_t n);
char* PortStrCpyN(char* dst, const char* src, int n);

// The game builds paths with backslashes ("scene\\bm201.bm2"); on POSIX
// filesystems (Android) they must become forward slashes. All stdio calls
// that take paths are routed through these wrappers.
char* Port_fullpath(char* absPath, const char* relPath, size_t maxLength);
#ifndef _WIN32
#define _fullpath Port_fullpath
#endif

FILE* Port_fopen(const char* path, const char* mode);
int   Port_remove(const char* path);
int   Port_rename(const char* oldp, const char* newp);
int   Port_mkdir(const char* path);
#ifndef PORT_NO_STDIO_REDIRECT
#define fopen  Port_fopen
#define remove Port_remove
#define rename Port_rename
#define _mkdir Port_mkdir
#endif

// ------------------------------------------------------------ API functions
// Implemented in port/compat/compat.cpp

void   OutputDebugString(const char* str);
DWORD  timeGetTime(void);
void   Sleep(DWORD ms);

// INI-file emulation (blupimania.ini) -> plain text file next to user data
DWORD  GetPrivateProfileString(const char* section, const char* key,
                               const char* def, char* out, DWORD outSize,
                               const char* file);
UINT   GetPrivateProfileInt(const char* section, const char* key,
                            int def, const char* file);
BOOL   WritePrivateProfileString(const char* section, const char* key,
                                 const char* value, const char* file);

// file helpers
BOOL   CopyFileA_shim(const char* src, const char* dst, BOOL failIfExists);
#define CopyFile CopyFileA_shim
BOOL   DeleteFileA_shim(const char* path);
#define DeleteFile DeleteFileA_shim
BOOL   CreateDirectoryA_shim(const char* path, void* sa);
#define CreateDirectory CreateDirectoryA_shim
BOOL   RemoveDirectoryA_shim(const char* path);
#define RemoveDirectory RemoveDirectoryA_shim

// message box: logged only (no modal UI on Android)
int    MessageBox(HWND hWnd, const char* text, const char* title, UINT type);
#define MB_OK               0x0000
#define MB_OKCANCEL         0x0001
#define MB_YESNO            0x0004
#define MB_ICONERROR        0x0010
#define MB_ICONWARNING      0x0030
#define MB_ICONSTOP         0x0010
#define IDOK                1
#define IDCANCEL            2
#define IDYES               6
#define IDNO                7

// cursor / input stubs
int    ShowCursor(BOOL bShow);
BOOL   SetCursorPos(int x, int y);
BOOL   GetCursorPos(POINT* p);
short  GetKeyState(int vk);
BOOL   GetKeyboardState(BYTE* state);

// process/thread priority stubs
#define NORMAL_PRIORITY_CLASS           0x20
#define HIGH_PRIORITY_CLASS             0x80
#define THREAD_PRIORITY_NORMAL          0
#define THREAD_PRIORITY_ABOVE_NORMAL    1
#define THREAD_PRIORITY_HIGHEST         2
HANDLE GetCurrentProcess(void);
HANDLE GetCurrentThread(void);
BOOL   SetPriorityClass(HANDLE process, DWORD priorityClass);
BOOL   SetThreadPriority(HANDLE thread, int priority);

// resource-string stub (CBot error strings) — always empty
int    LoadString(HINSTANCE inst, UINT id, char* buffer, int bufferMax);

// winmm timer resolution — meaningless under SDL
UINT   timeBeginPeriod(UINT period);
UINT   timeEndPeriod(UINT period);

// cursor stubs — the game draws its own mouse cursor ("nice mouse")
HCURSOR LoadCursor(HINSTANCE inst, const char* name);
HCURSOR SetCursor(HCURSOR cursor);
#define IDC_ARROW   ((const char*)32512)
#define IDC_IBEAM   ((const char*)32513)
#define IDC_WAIT    ((const char*)32514)
#define MAKEINTRESOURCE(i) ((const char*)(uintptr_t)(i))

// clipboard (edit.cpp cut/copy/paste) — backed by SDL_SetClipboardText
typedef HANDLE HGLOBAL;
#define GMEM_DDESHARE 0x2000
#define GMEM_MOVEABLE 0x0002
#define CF_TEXT       1
HGLOBAL GlobalAlloc(UINT flags, size_t size);
void*   GlobalLock(HGLOBAL h);
BOOL    GlobalUnlock(HGLOBAL h);
HGLOBAL GlobalFree(HGLOBAL h);
BOOL    OpenClipboard(HWND hWnd);
BOOL    EmptyClipboard(void);
HANDLE  SetClipboardData(UINT format, HANDLE mem);
HANDLE  GetClipboardData(UINT format);
BOOL    CloseClipboard(void);

// registry stubs — always fail, callers fall back to defaults
LONG   RegOpenKeyEx(HKEY root, const char* sub, DWORD opt, DWORD sam, HKEY* out);
LONG   RegQueryValueEx(HKEY key, const char* name, DWORD* rsv, DWORD* type,
                       BYTE* data, DWORD* size);
LONG   RegCloseKey(HKEY key);
#define HKEY_LOCAL_MACHINE  ((HKEY)(uintptr_t)0x80000002)
#define HKEY_CURRENT_USER   ((HKEY)(uintptr_t)0x80000001)
#define KEY_READ            0x20019
#define ERROR_SUCCESS       0
#define REG_SZ              1
#define REG_DWORD           4

#ifdef __cplusplus
}
#endif

// ------------------------------------------------- Win32 file API (fopen based)

#define GENERIC_READ            0x80000000
#define GENERIC_WRITE           0x40000000
#define FILE_SHARE_READ         0x00000001
#define CREATE_ALWAYS           2
#define OPEN_EXISTING           3
#define FILE_ATTRIBUTE_NORMAL   0x80
#define INVALID_HANDLE_VALUE    ((HANDLE)(intptr_t)-1)

#ifdef __cplusplus
extern "C" {
#endif

HANDLE CreateFile(const char* name, DWORD access, DWORD share, void* sa,
                  DWORD disposition, DWORD flags, HANDLE tmpl);
BOOL   ReadFile(HANDLE h, void* buf, DWORD toRead, DWORD* read, void* ov);
BOOL   WriteFile(HANDLE h, const void* buf, DWORD toWrite, DWORD* written, void* ov);
BOOL   CloseHandle(HANDLE h);

#ifdef __cplusplus
}
#endif

// -------------------------------------------------------- virtual key codes

#define VK_LBUTTON        0x01
#define VK_RBUTTON        0x02
#define VK_CANCEL         0x03
#define VK_MBUTTON        0x04
#define VK_BACK           0x08
#define VK_TAB            0x09
#define VK_CLEAR          0x0C
#define VK_RETURN         0x0D
#define VK_SHIFT          0x10
#define VK_CONTROL        0x11
#define VK_MENU           0x12
#define VK_PAUSE          0x13
#define VK_CAPITAL        0x14
#define VK_ESCAPE         0x1B
#define VK_SPACE          0x20
#define VK_PRIOR          0x21
#define VK_NEXT           0x22
#define VK_END            0x23
#define VK_HOME           0x24
#define VK_LEFT           0x25
#define VK_UP             0x26
#define VK_RIGHT          0x27
#define VK_DOWN           0x28
#define VK_SELECT         0x29
#define VK_PRINT          0x2A
#define VK_EXECUTE        0x2B
#define VK_SNAPSHOT       0x2C
#define VK_INSERT         0x2D
#define VK_DELETE         0x2E
#define VK_HELP           0x2F
#define VK_LWIN           0x5B
#define VK_RWIN           0x5C
#define VK_APPS           0x5D
#define VK_NUMPAD0        0x60
#define VK_NUMPAD1        0x61
#define VK_NUMPAD2        0x62
#define VK_NUMPAD3        0x63
#define VK_NUMPAD4        0x64
#define VK_NUMPAD5        0x65
#define VK_NUMPAD6        0x66
#define VK_NUMPAD7        0x67
#define VK_NUMPAD8        0x68
#define VK_NUMPAD9        0x69
#define VK_MULTIPLY       0x6A
#define VK_ADD            0x6B
#define VK_SEPARATOR      0x6C
#define VK_SUBTRACT       0x6D
#define VK_DECIMAL        0x6E
#define VK_DIVIDE         0x6F
#define VK_F1             0x70
#define VK_F2             0x71
#define VK_F3             0x72
#define VK_F4             0x73
#define VK_F5             0x74
#define VK_F6             0x75
#define VK_F7             0x76
#define VK_F8             0x77
#define VK_F9             0x78
#define VK_F10            0x79
#define VK_F11            0x7A
#define VK_F12            0x7B
#define VK_F13            0x7C
#define VK_F14            0x7D
#define VK_F15            0x7E
#define VK_F16            0x7F
#define VK_F17            0x80
#define VK_F18            0x81
#define VK_F19            0x82
#define VK_F20            0x83
#define VK_F21            0x84
#define VK_F22            0x85
#define VK_F23            0x86
#define VK_F24            0x87
#define VK_ATTN           0xF6
#define VK_CRSEL          0xF7
#define VK_EXSEL          0xF8
#define VK_EREOF          0xF9
#define VK_PLAY           0xFA
#define VK_ZOOM           0xFB
#define VK_NONAME         0xFC
#define VK_PA1            0xFD
#define VK_OEM_CLEAR      0xFE
#define VK_OEM_8          0xDF
#define VK_OEM_AX         0xE1
#define VK_OEM_102        0xE2
#define VK_PROCESSKEY     0xE5
#define VK_CONVERT        0x1C
#define VK_NONCONVERT     0x1D
#define VK_ACCEPT         0x1E
#define VK_MODECHANGE     0x1F
#define VK_JUNJA          0x17
#define VK_FINAL          0x18
#define VK_HANJA          0x19
#define VK_KANJI          0x19
#define VK_KANA           0x15
#define VK_HANGUL         0x15
#define VK_SLEEP          0x5F
#define VK_NUMLOCK        0x90
#define VK_SCROLL         0x91
#define VK_LSHIFT         0xA0
#define VK_RSHIFT         0xA1
#define VK_LCONTROL       0xA2
#define VK_RCONTROL       0xA3
#define VK_LMENU          0xA4
#define VK_RMENU          0xA5
#define VK_OEM_1          0xBA
#define VK_OEM_PLUS       0xBB
#define VK_OEM_COMMA      0xBC
#define VK_OEM_MINUS      0xBD
#define VK_OEM_PERIOD     0xBE
#define VK_OEM_2          0xBF
#define VK_OEM_3          0xC0
#define VK_OEM_4          0xDB
#define VK_OEM_5          0xDC
#define VK_OEM_6          0xDD
#define VK_OEM_7          0xDE

// window messages — referenced by event translation code; values arbitrary
#define WM_NULL           0x0000
#define WM_CREATE         0x0001
#define WM_DESTROY        0x0002
#define WM_MOVE           0x0003
#define WM_SIZE           0x0005
#define WM_ACTIVATE       0x0006
#define WM_SETFOCUS       0x0007
#define WM_KILLFOCUS      0x0008
#define WM_PAINT          0x000F
#define WM_CLOSE          0x0010
#define WM_QUIT           0x0012
#define WM_ACTIVATEAPP    0x001C
#define WM_SETCURSOR      0x0020
#define WM_KEYDOWN        0x0100
#define WM_KEYUP          0x0101
#define WM_CHAR           0x0102
#define WM_SYSKEYDOWN     0x0104
#define WM_SYSKEYUP       0x0105
#define WM_SYSCOMMAND     0x0112
#define WM_MOUSEMOVE      0x0200
#define WM_LBUTTONDOWN    0x0201
#define WM_LBUTTONUP      0x0202
#define WM_RBUTTONDOWN    0x0204
#define WM_RBUTTONUP      0x0205
#define WM_MBUTTONDOWN    0x0207
#define WM_MBUTTONUP      0x0208
#define WM_MOUSEWHEEL     0x020A

#endif // _PORT_WINDOWS_H_
