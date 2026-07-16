// sdljoystick.cpp — joystick.h implemented over SDL2 (replaces src/joystick.cpp).

#include <windows.h>
#include <dinput.h>
#include <SDL.h>

#include "joystick.h"

static SDL_Joystick* g_joystick = NULL;

BOOL InitDirectInput(HINSTANCE hInst, HWND hWnd, BOOL &bFFB)
{
    (void)hInst; (void)hWnd;
    bFFB = FALSE;
    if (SDL_NumJoysticks() < 1) return FALSE;
    g_joystick = SDL_JoystickOpen(0);
    return g_joystick != NULL;
}

BOOL SetAcquire(BOOL bActive)
{
    (void)bActive;
    return TRUE;
}

BOOL UpdateInputState(DIJOYSTATE &js)
{
    if (g_joystick == NULL) return FALSE;
    SDL_JoystickUpdate();

    memset(&js, 0, sizeof(js));
    int axes = SDL_JoystickNumAxes(g_joystick);
    if (axes > 0) js.lX = SDL_JoystickGetAxis(g_joystick, 0) * 1000 / 32768;
    if (axes > 1) js.lY = SDL_JoystickGetAxis(g_joystick, 1) * 1000 / 32768;
    if (axes > 2) js.lZ = SDL_JoystickGetAxis(g_joystick, 2) * 1000 / 32768;
    if (axes > 3) js.lRz = SDL_JoystickGetAxis(g_joystick, 3) * 1000 / 32768;

    int buttons = SDL_JoystickNumButtons(g_joystick);
    if (buttons > 32) buttons = 32;
    for (int i = 0; i < buttons; i++)
        js.rgbButtons[i] = SDL_JoystickGetButton(g_joystick, i) ? 0x80 : 0;

    return TRUE;
}

BOOL FreeDirectInput()
{
    if (g_joystick != NULL)
    {
        SDL_JoystickClose(g_joystick);
        g_joystick = NULL;
    }
    return TRUE;
}

BOOL SetJoyForces(float forceX, float forceY)
{
    (void)forceX; (void)forceY;
    return TRUE;
}
