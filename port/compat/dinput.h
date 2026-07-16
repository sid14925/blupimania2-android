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
