// tchar.h — ANSI-only TCHAR helpers.

#ifndef _PORT_TCHAR_H_
#define _PORT_TCHAR_H_

#if defined(_WIN32) && !defined(__ANDROID__)
#include_next <tchar.h>
#else
#include <string.h>
#define _T(x)      x
#define _tcsrchr   strrchr
#define _tcslen    strlen
#define _tcscpy    strcpy
#define _tcscat    strcat
#endif

#endif // _PORT_TCHAR_H_
