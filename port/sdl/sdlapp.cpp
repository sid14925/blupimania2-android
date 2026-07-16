// sdlapp.cpp — CD3DApplication reimplemented on SDL2 + OpenGL (ES) 2.
// Replaces src/d3dapp.cpp, src/d3dframe.cpp, src/d3denum.cpp and winmain.cpp.
// Touch input is translated to the game's mouse/keyboard event model:
//   one finger        = left mouse button (tap-to-move / UI)
//   two-finger tap    = right mouse button
//   two-finger pinch  = mouse wheel (camera zoom)

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <dinput.h>

#include <SDL.h>

#include "struct.h"
#include "D3DTextr.h"
#include "D3DEngine.h"
#include "language.h"
#include "event.h"
#include "profile.h"
#include "iman.h"
#include "restext.h"
#include "math3d.h"
#include "joystick.h"
#include "robotmain.h"
#include "sound.h"
#include "D3DApp.h"

#include "../gl/glapi.h"

#define MAX_STEP        0.2f    // maximum time for one step

// desktop default window size; Android always uses the full display
#define DESKTOP_DX      1024
#define DESKTOP_DY      768

static CD3DApplication* g_pD3DApp = NULL;

static SDL_Window*      g_window = NULL;
static SDL_GLContext    g_glContext = NULL;
static int              g_winWidth = DESKTOP_DX;
static int              g_winHeight = DESKTOP_DY;
static BOOL             g_bQuit = FALSE;

// touch gesture state
static BOOL             g_bFinger0Down = FALSE;
static SDL_FingerID     g_finger0 = 0;
static int              g_fingerCount = 0;
static float            g_pinchDist = 0.0f;
static BOOL             g_bPinching = FALSE;
static float            g_gestureLastX = -1.0f;
static float            g_gesturePanAcc = 0.0f;

static float AxeLimit(float value)
{
    if (value < -1.0f) value = -1.0f;
    if (value >  1.0f) value =  1.0f;
    return value;
}

static float PortNeutral(float value, float dead)
{
    if (fabsf(value) <= dead) return 0.0f;
    if (value > 0.0f) return (value - dead) / (1.0f - dead);
    return (value + dead) / (1.0f - dead);
}

// ---------------------------------------------------------------------------
// SDL keycode -> Windows virtual key translation
// ---------------------------------------------------------------------------

static int SDLKeyToVK(SDL_Keycode key)
{
    if (key >= SDLK_a && key <= SDLK_z) return 'A' + (key - SDLK_a);
    if (key >= SDLK_0 && key <= SDLK_9) return '0' + (key - SDLK_0);
    if (key >= SDLK_F1 && key <= SDLK_F12) return VK_F1 + (key - SDLK_F1);
    if (key >= SDLK_KP_1 && key <= SDLK_KP_9) return VK_NUMPAD1 + (key - SDLK_KP_1);

    switch (key)
    {
        case SDLK_KP_0:      return VK_NUMPAD0;
        case SDLK_LEFT:      return VK_LEFT;
        case SDLK_RIGHT:     return VK_RIGHT;
        case SDLK_UP:        return VK_UP;
        case SDLK_DOWN:      return VK_DOWN;
        case SDLK_RETURN:    return VK_RETURN;
        case SDLK_KP_ENTER:  return VK_RETURN;
        case SDLK_ESCAPE:    return VK_ESCAPE;
        case SDLK_SPACE:     return VK_SPACE;
        case SDLK_BACKSPACE: return VK_BACK;
        case SDLK_TAB:       return VK_TAB;
        case SDLK_DELETE:    return VK_DELETE;
        case SDLK_INSERT:    return VK_INSERT;
        case SDLK_HOME:      return VK_HOME;
        case SDLK_END:       return VK_END;
        case SDLK_PAGEUP:    return VK_PRIOR;
        case SDLK_PAGEDOWN:  return VK_NEXT;
        case SDLK_LSHIFT:    return VK_SHIFT;
        case SDLK_RSHIFT:    return VK_SHIFT;
        case SDLK_LCTRL:     return VK_CONTROL;
        case SDLK_RCTRL:     return VK_CONTROL;
        case SDLK_LALT:      return VK_MENU;
        case SDLK_RALT:      return VK_MENU;
        case SDLK_PAUSE:     return VK_PAUSE;
        case SDLK_KP_PLUS:   return VK_ADD;
        case SDLK_KP_MINUS:  return VK_SUBTRACT;
        case SDLK_KP_MULTIPLY: return VK_MULTIPLY;
        case SDLK_KP_DIVIDE: return VK_DIVIDE;
        case SDLK_KP_PERIOD: return VK_DECIMAL;
        case SDLK_CAPSLOCK:  return VK_CAPITAL;
        case SDLK_NUMLOCKCLEAR: return VK_NUMLOCK;
        case SDLK_COMMA:     return VK_OEM_COMMA;
        case SDLK_PERIOD:    return VK_OEM_PERIOD;
        case SDLK_MINUS:     return VK_OEM_MINUS;
        case SDLK_PLUS:      return VK_OEM_PLUS;
        case SDLK_AC_BACK:   return VK_ESCAPE;   // Android back button
        default: break;
    }
    return 0;
}

// ---------------------------------------------------------------------------
// construction
// ---------------------------------------------------------------------------

CD3DApplication::CD3DApplication()
{
    int i;

    m_iMan  = new CInstanceManager();
    m_event = new CEvent(m_iMan);

    m_pD3DEngine = 0;
    m_pRobotMain = 0;
    m_pSound     = 0;
    m_pFramework = 0;
    m_instance   = 0;
    m_hWnd       = 0;
    m_pDD        = 0;
    m_pD3D       = 0;
    m_pD3DDevice = 0;

    m_CDpath[0] = 0;

    m_pddsRenderTarget = 0;
    m_pddsDepthBuffer  = 0;

    m_keyState = 0;
    m_axeKeyX = 0.0f;
    m_axeKeyY = 0.0f;
    m_axeKeyZ = 0.0f;
    m_axeKeyW = 0.0f;
    m_axeJoy = D3DVECTOR(0.0f, 0.0f, 0.0f);

    m_vidMemTotal  = 128*1024*1024;     // report plenty of texture memory
    m_bActive      = FALSE;
    m_bActivateApp = FALSE;
    m_bReady       = FALSE;
    m_joystick     = 0;
    m_FFBforce     = 1.0f;
    m_bFFB         = FALSE;
    m_aTime        = 0.0f;

    for (i = 0; i < 32; i++) m_bJoyButton[i] = FALSE;
    m_bJoyLeft  = FALSE;
    m_bJoyRight = FALSE;
    m_bJoyUp    = FALSE;
    m_bJoyDown  = FALSE;

    m_strWindowTitle  = (TCHAR*)"BlupiMania 2";
    m_bAppUseZBuffer  = TRUE;
    m_bAppUseStereo   = FALSE;
    m_bShowStats      = FALSE;
    m_bDebugMode      = FALSE;
    m_bAudioState     = TRUE;
    m_bAudioTrack     = FALSE;      // no CD audio in the port
    m_fnConfirmDevice = 0;
    m_mshMouseWheel   = 0;

    ResetKey();

    g_pD3DApp = this;
}

CD3DApplication::~CD3DApplication()
{
    g_pD3DApp = NULL;
    delete m_event;
    delete m_iMan;
}

char* CD3DApplication::RetCDpath()
{
    return m_CDpath;
}

Error CD3DApplication::RegQuery()   { return ERR_OK; }    // no CD check
Error CD3DApplication::AudioQuery() { return ERR_OK; }

Error CD3DApplication::CheckMistery(char *strCmdLine)
{
    if (strstr(strCmdLine, "-debug") != 0)
    {
        m_bShowStats = TRUE;
        SetDebugMode(TRUE);
    }
    if (strstr(strCmdLine, "-audiostate") != 0) m_bAudioState = FALSE;
    m_bAudioTrack = FALSE;
    m_CDpath[0] = 0;
    return ERR_OK;
}

int  CD3DApplication::GetVidMemTotal() { return (int)m_vidMemTotal; }
BOOL CD3DApplication::IsVideo8MB()     { return FALSE; }
BOOL CD3DApplication::IsVideo32MB()    { return TRUE; }

void CD3DApplication::SetShowStat(BOOL bShow) { m_bShowStats = bShow; }
BOOL CD3DApplication::RetShowStat()           { return m_bShowStats; }

void CD3DApplication::SetDebugMode(BOOL bMode)
{
    m_bDebugMode = bMode;
    D3DTextr_SetDebugMode(m_bDebugMode);
}
BOOL CD3DApplication::RetDebugMode() { return m_bDebugMode; }

HRESULT CD3DApplication::ConfirmDevice(DDCAPS* pddDriverCaps,
                                       D3DDEVICEDESC7* pd3dDeviceDesc)
{
    (void)pddDriverCaps; (void)pd3dDeviceDesc;
    return S_OK;
}

// ---------------------------------------------------------------------------
// window / GL creation
// ---------------------------------------------------------------------------

HRESULT CD3DApplication::Initialize3DEnvironment()
{
    m_pD3DDevice = new IDirect3DDevice7();
    if (FAILED(m_pD3DDevice->InitDevice(g_winWidth, g_winHeight)))
        return E_FAIL;

    m_pD3DEngine->SetD3DDevice(m_pD3DDevice);

    // render target description consulted by ConvPosToInterface & the engine
    memset(&m_ddsdRenderTarget, 0, sizeof(m_ddsdRenderTarget));
    m_ddsdRenderTarget.dwSize   = sizeof(m_ddsdRenderTarget);
    m_ddsdRenderTarget.dwWidth  = (DWORD)g_winWidth;
    m_ddsdRenderTarget.dwHeight = (DWORD)g_winHeight;

    if (FAILED(m_pD3DEngine->InitDeviceObjects()))
        return E_FAIL;

    return S_OK;
}

HRESULT CD3DApplication::Change3DEnvironment() { return S_OK; }
HRESULT CD3DApplication::CreateZBuffer(GUID*)  { return S_OK; }

HRESULT CD3DApplication::Create(HINSTANCE hInst, TCHAR* strCmdLine)
{
    HRESULT hr;
    int     iValue;
    float   fValue;

    (void)strCmdLine;
    m_instance = hInst;

    fprintf(stderr, "[port] SDL_Init...\n"); fflush(stderr);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) != 0)
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return E_FAIL;
    }

#ifdef __ANDROID__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    Uint32 winFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_RESIZABLE;
#else
    Uint32 winFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
#endif
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    g_window = SDL_CreateWindow("BlupiMania 2",
                                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                DESKTOP_DX, DESKTOP_DY, winFlags);
    if (g_window == NULL)
    {
        // fall back to a 16-bit depth buffer
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
        g_window = SDL_CreateWindow("BlupiMania 2",
                                    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                    DESKTOP_DX, DESKTOP_DY, winFlags);
    }
    if (g_window == NULL)
    {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        return E_FAIL;
    }

    fprintf(stderr, "[port] window created\n"); fflush(stderr);
    g_glContext = SDL_GL_CreateContext(g_window);
    if (g_glContext == NULL)
    {
        SDL_Log("SDL_GL_CreateContext failed: %s", SDL_GetError());
        return E_FAIL;
    }
    SDL_GL_SetSwapInterval(1);
    SDL_GL_GetDrawableSize(g_window, &g_winWidth, &g_winHeight);

    fprintf(stderr, "[port] GL context created\n"); fflush(stderr);
    if (!PortGLInit())
    {
        SDL_Log("OpenGL function loading failed");
        return E_FAIL;
    }

    fprintf(stderr, "[port] GL functions loaded\n"); fflush(stderr);
    m_hWnd = (HWND)g_window;

    // create the engine and game objects in the original order
    if ((m_pD3DEngine = new CD3DEngine(m_iMan, this)) == NULL)
        return E_OUTOFMEMORY;
    SetEngine(m_pD3DEngine);
    fprintf(stderr, "[port] engine created\n"); fflush(stderr);

    fprintf(stderr, "[port] OneTimeSceneInit...\n"); fflush(stderr);
    if (FAILED(hr = m_pD3DEngine->OneTimeSceneInit()))
        return hr;

    fprintf(stderr, "[port] OneTimeSceneInit done, creating CSound\n"); fflush(stderr);
    if ((m_pSound = new CSound(m_iMan)) == NULL)
        return E_OUTOFMEMORY;

    fprintf(stderr, "[port] CSound created, creating CRobotMain\n"); fflush(stderr);
    if ((m_pRobotMain = new CRobotMain(m_iMan)) == NULL)
        return E_OUTOFMEMORY;

    fprintf(stderr, "[port] sound+robotmain created\n"); fflush(stderr);
    m_pSound->SetDebugMode(m_bDebugMode);
    m_pSound->Create(m_hWnd, TRUE);
    m_pSound->CacheAll();
    m_pSound->SetState(m_bAudioState);
    m_pSound->SetAudioTrack(m_bAudioTrack);
    m_pSound->SetCDpath(m_CDpath);

    fprintf(stderr, "[port] sound ready, init 3D env...\n"); fflush(stderr);
    if (FAILED(hr = Initialize3DEnvironment()))
    {
        Cleanup3DEnvironment();
        return E_FAIL;
    }

    if (!GetProfileInt("Setup", "ObjectDirty", iValue))
    {
        m_pD3DEngine->FirstExecuteAdapt(TRUE);
    }

    if (GetProfileFloat("Setup", "JoystickForce", fValue)) m_pD3DEngine->SetForce(fValue);
    if (GetProfileInt("Setup", "JoystickFFB", iValue))     m_pD3DEngine->SetFFB(iValue);
    if (GetProfileInt("Setup", "UseJoystick", iValue))     m_pD3DEngine->SetJoystick(iValue);

    fprintf(stderr, "[port] 3D env ready, ChangePhase...\n"); fflush(stderr);
    m_pRobotMain->ChangePhase(PHASE_WELCOME3);
    m_pD3DEngine->TimeInit();

    fprintf(stderr, "[port] phase changed OK\n"); fflush(stderr);
    m_bActive      = TRUE;
    m_bActivateApp = TRUE;
    m_bReady       = TRUE;
    return S_OK;
}

// ---------------------------------------------------------------------------
// event dispatch (mirrors the original WndProc)
// ---------------------------------------------------------------------------

void CD3DApplication::SetMousePos(FPOINT pos)
{
    m_mousePos = pos;
    int x = (int)(pos.x * g_winWidth);
    int y = (int)((1.0f - pos.y) * g_winHeight);
    SDL_WarpMouseInWindow(g_window, x, y);
}

void CD3DApplication::SetMouseType(D3DMouse type)  { (void)type; }
BOOL CD3DApplication::RetNiceMouseCap()            { return TRUE; }
void CD3DApplication::SetMouseCapture()            { SDL_CaptureMouse(SDL_TRUE); }
void CD3DApplication::ReleaseMouseCapture()        { SDL_CaptureMouse(SDL_FALSE); }

FPOINT CD3DApplication::ConvPosToInterface(HWND hWnd, LPARAM lParam)
{
    (void)hWnd;
    FPOINT pos;
    int x = (int)(short)LOWORD(lParam);
    int y = (int)(short)HIWORD(lParam);
    pos.x = (float)x / (float)g_winWidth;
    pos.y = 1.0f - (float)y / (float)g_winHeight;
    return pos;
}

// forwards a Win32-style message through the original game event path
static void DispatchGameMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CD3DApplication* app = g_pD3DApp;
    if (app == NULL) return;

    Event event;
    ZeroMemory(&event, sizeof(Event));

    if (uMsg == WM_LBUTTONDOWN) event.event = EVENT_LBUTTONDOWN;
    if (uMsg == WM_RBUTTONDOWN) event.event = EVENT_RBUTTONDOWN;
    if (uMsg == WM_LBUTTONUP)   event.event = EVENT_LBUTTONUP;
    if (uMsg == WM_RBUTTONUP)   event.event = EVENT_RBUTTONUP;
    if (uMsg == WM_MOUSEMOVE)   event.event = EVENT_MOUSEMOVE;
    if (uMsg == WM_KEYDOWN)     event.event = EVENT_KEYDOWN;
    if (uMsg == WM_KEYUP)       event.event = EVENT_KEYUP;
    if (uMsg == WM_CHAR)        event.event = EVENT_CHAR;

    event.param = (long)wParam;
    event.axeX = AxeLimit(app->m_axeKeyX + app->m_axeJoy.x);
    event.axeY = AxeLimit(app->m_axeKeyY + app->m_axeJoy.y);
    event.axeZ = AxeLimit(app->m_axeKeyZ + app->m_axeJoy.z);
    event.axeW = app->m_axeKeyW;
    event.keyState = app->m_keyState;

    if (uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN ||
        uMsg == WM_LBUTTONUP   || uMsg == WM_RBUTTONUP   ||
        uMsg == WM_MOUSEMOVE)
    {
        event.pos = app->ConvPosToInterface(NULL, lParam);
        app->m_mousePos = event.pos;
        app->m_pD3DEngine->SetMousePos(event.pos);
    }

    if (uMsg == WM_MOUSEWHEEL)
    {
        event.event = EVENT_KEYDOWN;
        event.pos = app->m_mousePos;
        short move = (short)HIWORD(wParam);
        if (move > 0) event.param = VK_WHEELUP;
        if (move < 0) event.param = VK_WHEELDOWN;
    }

    if (event.event == EVENT_KEYDOWN ||
        event.event == EVENT_KEYUP   ||
        event.event == EVENT_CHAR)
    {
        if (event.param == 0) event.event = EVENT_NULL;
    }

    if (app->m_pRobotMain != 0 && event.event != 0)
    {
        app->m_pRobotMain->EventProcess(event);
    }
    if (app->m_pD3DEngine != 0)
    {
        app->m_pD3DEngine->MsgProc(NULL, uMsg, wParam, lParam);
    }
    app->MsgProc(NULL, uMsg, wParam, lParam);
}

LRESULT CD3DApplication::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    (void)hWnd; (void)lParam;

    switch (uMsg)
    {
        case WM_KEYDOWN:
            if (wParam == m_key[KEYRANK_UP   ][0]) m_axeKeyY =  1.0f;
            if (wParam == m_key[KEYRANK_UP   ][1]) m_axeKeyY =  1.0f;
            if (wParam == m_key[KEYRANK_DOWN ][0]) m_axeKeyY = -1.0f;
            if (wParam == m_key[KEYRANK_DOWN ][1]) m_axeKeyY = -1.0f;
            if (wParam == m_key[KEYRANK_LEFT ][0]) m_axeKeyX = -1.0f;
            if (wParam == m_key[KEYRANK_LEFT ][1]) m_axeKeyX = -1.0f;
            if (wParam == m_key[KEYRANK_RIGHT][0]) m_axeKeyX =  1.0f;
            if (wParam == m_key[KEYRANK_RIGHT][1]) m_axeKeyX =  1.0f;
            if (wParam == m_key[KEYRANK_NEAR ][0]) m_keyState |= KS_NUMPLUS;
            if (wParam == m_key[KEYRANK_NEAR ][1]) m_keyState |= KS_NUMPLUS;
            if (wParam == m_key[KEYRANK_AWAY ][0]) m_keyState |= KS_NUMMINUS;
            if (wParam == m_key[KEYRANK_AWAY ][1]) m_keyState |= KS_NUMMINUS;
            if (wParam == VK_PRIOR)   m_keyState |= KS_PAGEUP;
            if (wParam == VK_NEXT)    m_keyState |= KS_PAGEDOWN;
            if (wParam == VK_NUMPAD8) m_keyState |= KS_NUMUP;
            if (wParam == VK_NUMPAD2) m_keyState |= KS_NUMDOWN;
            if (wParam == VK_NUMPAD4) m_keyState |= KS_NUMLEFT;
            if (wParam == VK_NUMPAD6) m_keyState |= KS_NUMRIGHT;
            break;

        case WM_KEYUP:
            if (wParam == m_key[KEYRANK_UP   ][0]) m_axeKeyY = 0.0f;
            if (wParam == m_key[KEYRANK_UP   ][1]) m_axeKeyY = 0.0f;
            if (wParam == m_key[KEYRANK_DOWN ][0]) m_axeKeyY = 0.0f;
            if (wParam == m_key[KEYRANK_DOWN ][1]) m_axeKeyY = 0.0f;
            if (wParam == m_key[KEYRANK_LEFT ][0]) m_axeKeyX = 0.0f;
            if (wParam == m_key[KEYRANK_LEFT ][1]) m_axeKeyX = 0.0f;
            if (wParam == m_key[KEYRANK_RIGHT][0]) m_axeKeyX = 0.0f;
            if (wParam == m_key[KEYRANK_RIGHT][1]) m_axeKeyX = 0.0f;
            if (wParam == m_key[KEYRANK_NEAR ][0]) m_keyState &= ~KS_NUMPLUS;
            if (wParam == m_key[KEYRANK_NEAR ][1]) m_keyState &= ~KS_NUMPLUS;
            if (wParam == m_key[KEYRANK_AWAY ][0]) m_keyState &= ~KS_NUMMINUS;
            if (wParam == m_key[KEYRANK_AWAY ][1]) m_keyState &= ~KS_NUMMINUS;
            if (wParam == VK_PRIOR)   m_keyState &= ~KS_PAGEUP;
            if (wParam == VK_NEXT)    m_keyState &= ~KS_PAGEDOWN;
            if (wParam == VK_NUMPAD8) m_keyState &= ~KS_NUMUP;
            if (wParam == VK_NUMPAD2) m_keyState &= ~KS_NUMDOWN;
            if (wParam == VK_NUMPAD4) m_keyState &= ~KS_NUMLEFT;
            if (wParam == VK_NUMPAD6) m_keyState &= ~KS_NUMRIGHT;
            break;

        case WM_LBUTTONDOWN: m_keyState |= KS_MLEFT;  break;
        case WM_RBUTTONDOWN: m_keyState |= KS_MRIGHT; break;
        case WM_LBUTTONUP:   m_keyState &= ~KS_MLEFT;  break;
        case WM_RBUTTONUP:   m_keyState &= ~KS_MRIGHT; break;
    }
    return 0;
}

// ---------------------------------------------------------------------------
// main loop
// ---------------------------------------------------------------------------

static LPARAM MakeMousePos(float xNorm, float yNorm)
{
    int x = (int)(xNorm * g_winWidth);
    int y = (int)(yNorm * g_winHeight);
    return (LPARAM)MAKELONG(x, y);
}

static void HandleSDLEvent(const SDL_Event& e)
{
    CD3DApplication* app = g_pD3DApp;

    switch (e.type)
    {
        case SDL_QUIT:
            g_bQuit = TRUE;
            break;

        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
            int vk = SDLKeyToVK(e.key.keysym.sym);
            if (vk == 0) break;

            if (e.key.keysym.mod & (KMOD_LSHIFT|KMOD_RSHIFT))
                app->m_keyState |= KS_SHIFT;
            else
                app->m_keyState &= ~KS_SHIFT;
            if (e.key.keysym.mod & (KMOD_LCTRL|KMOD_RCTRL))
                app->m_keyState |= KS_CONTROL;
            else
                app->m_keyState &= ~KS_CONTROL;

            DispatchGameMsg(e.type == SDL_KEYDOWN ? WM_KEYDOWN : WM_KEYUP, vk, 0);
            break;
        }

        case SDL_TEXTINPUT:
        {
            const char* t = e.text.text;
            while (*t != 0)
            {
                unsigned char c = (unsigned char)*t++;
                if (c < 0x80)   // ASCII only; the game uses 8-bit charsets
                    DispatchGameMsg(WM_CHAR, c, 0);
            }
            break;
        }

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        {
            BOOL bDown = (e.type == SDL_MOUSEBUTTONDOWN);
            LPARAM lp = (LPARAM)MAKELONG(e.button.x, e.button.y);
            if (e.button.button == SDL_BUTTON_LEFT)
                DispatchGameMsg(bDown ? WM_LBUTTONDOWN : WM_LBUTTONUP, 0, lp);
            else if (e.button.button == SDL_BUTTON_RIGHT)
                DispatchGameMsg(bDown ? WM_RBUTTONDOWN : WM_RBUTTONUP, 0, lp);
            break;
        }

        case SDL_MOUSEMOTION:
            if (e.motion.which != SDL_TOUCH_MOUSEID)
                DispatchGameMsg(WM_MOUSEMOVE, 0, (LPARAM)MAKELONG(e.motion.x, e.motion.y));
            break;

        case SDL_MOUSEWHEEL:
            DispatchGameMsg(WM_MOUSEWHEEL,
                            (WPARAM)MAKELONG(0, e.wheel.y > 0 ? 120 : -120), 0);
            break;

        // ------------------------------------------------ touch input
        case SDL_FINGERDOWN:
        {
            g_fingerCount++;
            if (g_fingerCount == 1)
            {
                g_bFinger0Down = TRUE;
                g_finger0 = e.tfinger.fingerId;
                LPARAM lp = MakeMousePos(e.tfinger.x, e.tfinger.y);
                DispatchGameMsg(WM_MOUSEMOVE, 0, lp);
                DispatchGameMsg(WM_LBUTTONDOWN, 0, lp);
            }
            else if (g_fingerCount == 2)
            {
                // second finger cancels the left press -> becomes a gesture
                if (g_bFinger0Down)
                {
                    LPARAM lp = MakeMousePos(e.tfinger.x, e.tfinger.y);
                    DispatchGameMsg(WM_LBUTTONUP, 0, lp);
                    g_bFinger0Down = FALSE;
                }
                g_bPinching = TRUE;
                g_pinchDist = 0.0f;
            }
            break;
        }

        case SDL_FINGERUP:
        {
            if (g_fingerCount > 0) g_fingerCount--;
            if (e.tfinger.fingerId == g_finger0 && g_bFinger0Down)
            {
                LPARAM lp = MakeMousePos(e.tfinger.x, e.tfinger.y);
                DispatchGameMsg(WM_LBUTTONUP, 0, lp);
                g_bFinger0Down = FALSE;
            }
            if (g_fingerCount < 2)
            {
                g_bPinching = FALSE;
                g_gestureLastX = -1.0f;
                g_gesturePanAcc = 0.0f;
                g_pinchDist = 0.0f;
            }
            break;
        }

        case SDL_FINGERMOTION:
        {
            if (g_bFinger0Down && e.tfinger.fingerId == g_finger0)
            {
                DispatchGameMsg(WM_MOUSEMOVE, 0, MakeMousePos(e.tfinger.x, e.tfinger.y));
            }
            break;
        }

        case SDL_MULTIGESTURE:
        {
            if (e.mgesture.numFingers == 2)
            {
                static float s_lastX = -1.0f;

                // pinch -> camera zoom (the game's +/- keys)
                g_pinchDist += e.mgesture.dDist;
                while (g_pinchDist > 0.03f)
                {
                    DispatchGameMsg(WM_KEYDOWN, VK_ADD, 0);
                    DispatchGameMsg(WM_KEYUP, VK_ADD, 0);
                    g_pinchDist -= 0.03f;
                }
                while (g_pinchDist < -0.03f)
                {
                    DispatchGameMsg(WM_KEYDOWN, VK_SUBTRACT, 0);
                    DispatchGameMsg(WM_KEYUP, VK_SUBTRACT, 0);
                    g_pinchDist += 0.03f;
                }

                // two-finger horizontal drag -> camera rotation (mouse wheel)
                if (g_gestureLastX >= 0.0f)
                {
                    g_gesturePanAcc += e.mgesture.x - g_gestureLastX;
                    while (g_gesturePanAcc > 0.04f)
                    {
                        DispatchGameMsg(WM_MOUSEWHEEL, (WPARAM)MAKELONG(0, 120), 0);
                        g_gesturePanAcc -= 0.04f;
                    }
                    while (g_gesturePanAcc < -0.04f)
                    {
                        DispatchGameMsg(WM_MOUSEWHEEL, (WPARAM)MAKELONG(0, -120), 0);
                        g_gesturePanAcc += 0.04f;
                    }
                }
                g_gestureLastX = e.mgesture.x;
            }
            break;
        }

        case SDL_WINDOWEVENT:
            if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                SDL_GL_GetDrawableSize(g_window, &g_winWidth, &g_winHeight);
                if (app->m_pD3DDevice != NULL)
                    app->m_pD3DDevice->Resize(g_winWidth, g_winHeight);
                app->m_ddsdRenderTarget.dwWidth  = (DWORD)g_winWidth;
                app->m_ddsdRenderTarget.dwHeight = (DWORD)g_winHeight;
            }
            else if (e.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
            {
                app->m_bActivateApp = FALSE;
            }
            else if (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
            {
                app->m_bActivateApp = TRUE;
            }
            break;

        case SDL_APP_WILLENTERBACKGROUND:
            app->m_bActive = FALSE;
            break;
        case SDL_APP_DIDENTERFOREGROUND:
            app->m_bActive = TRUE;
            if (app->m_pD3DEngine != NULL) app->m_pD3DEngine->TimeInit();
            break;

        default:
            break;
    }
}

void CD3DApplication::StepSimul(float rTime)
{
    Event event;

    if (rTime == 0.0f) return;
    if (m_pRobotMain == 0) return;

    ZeroMemory(&event, sizeof(Event));
    event.event = EVENT_FRAME;
    event.rTime = rTime;
    event.axeX = AxeLimit(m_axeKeyX + m_axeJoy.x);
    event.axeY = AxeLimit(m_axeKeyY + m_axeJoy.y);
    event.axeZ = AxeLimit(m_axeKeyZ + m_axeJoy.z);
    event.axeW = m_axeKeyW;
    event.keyState = m_keyState;

    m_pRobotMain->EventProcess(event);
}

// polls SDL joystick state through the DirectInput-shaped shim
static void PollJoystick(CD3DApplication* app)
{
    DIJOYSTATE js;
    if (app->m_joystick == 0) return;
    if (!UpdateInputState(js)) return;

    app->m_axeJoy.x =  js.lX/1000.0f + js.lRz/1000.0f;
    app->m_axeJoy.y = -js.lY/1000.0f;
    app->m_axeJoy.z = -js.rglSlider[0]/1000.0f;

    if (app->m_axeJoy.x > 0.5f && !app->m_bJoyRight)
    {
        app->m_bJoyRight = TRUE;
        DispatchGameMsg(WM_KEYDOWN, VK_JRIGHT, 0);
    }
    if (app->m_axeJoy.x < 0.3f && app->m_bJoyRight)  app->m_bJoyRight = FALSE;
    if (app->m_axeJoy.x < -0.5f && !app->m_bJoyLeft)
    {
        app->m_bJoyLeft = TRUE;
        DispatchGameMsg(WM_KEYDOWN, VK_JLEFT, 0);
    }
    if (app->m_axeJoy.x > -0.3f && app->m_bJoyLeft)  app->m_bJoyLeft = FALSE;
    if (app->m_axeJoy.y > 0.5f && !app->m_bJoyUp)
    {
        app->m_bJoyUp = TRUE;
        DispatchGameMsg(WM_KEYDOWN, VK_JUP, 0);
    }
    if (app->m_axeJoy.y < 0.3f && app->m_bJoyUp)     app->m_bJoyUp = FALSE;
    if (app->m_axeJoy.y < -0.5f && !app->m_bJoyDown)
    {
        app->m_bJoyDown = TRUE;
        DispatchGameMsg(WM_KEYDOWN, VK_JDOWN, 0);
    }
    if (app->m_axeJoy.y > -0.3f && app->m_bJoyDown)  app->m_bJoyDown = FALSE;

    app->m_axeJoy.y = PortNeutral(app->m_axeJoy.y, 0.2f);
    app->m_axeJoy.z = PortNeutral(app->m_axeJoy.z, 0.2f);

    for (int i = 0; i < 32; i++)
    {
        if (js.rgbButtons[i] != 0 && !app->m_bJoyButton[i])
        {
            app->m_bJoyButton[i] = TRUE;
            DispatchGameMsg(WM_KEYDOWN, VK_BUTTON1+i, 0);
        }
        if (js.rgbButtons[i] == 0 && app->m_bJoyButton[i])
        {
            app->m_bJoyButton[i] = FALSE;
            DispatchGameMsg(WM_KEYUP, VK_BUTTON1+i, 0);
        }
    }
}

HRESULT CD3DApplication::Render3DEnvironment()
{
    HRESULT hr;
    float rTime;

    rTime = m_pD3DEngine->TimeGet();
    if (rTime > MAX_STEP) rTime = MAX_STEP;
    m_aTime += rTime;

    if (FAILED(hr = m_pD3DEngine->FrameMove(rTime)))
        return hr;

    StepSimul(rTime);

    if (FAILED(hr = m_pD3DEngine->Render()))
        return hr;

    DrawSuppl();

    SDL_GL_SwapWindow(g_window);
    return S_OK;
}

INT CD3DApplication::Run()
{
    while (!g_bQuit)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            HandleSDLEvent(e);
        }
        if (g_bQuit) break;

        // drain the game's internal event queue
        Event event;
        while (m_event->GetEvent(event))
        {
            if (event.event == EVENT_QUIT)
            {
                m_pSound->StopMusic();
                Cleanup3DEnvironment();
                return 0;
            }
            m_pRobotMain->EventProcess(event);
        }

        PollJoystick(this);

        if (m_bActive && m_bReady)
        {
            if (FAILED(Render3DEnvironment()))
                g_bQuit = TRUE;
        }
        else
        {
            SDL_Delay(50);
        }
    }
    Cleanup3DEnvironment();
    return 0;
}

VOID CD3DApplication::Pause(BOOL bPause)
{
    if (bPause)
    {
        m_pD3DEngine->TimeEnterGel();
    }
    else
    {
        m_pD3DEngine->TimeExitGel();
    }
}

LRESULT CD3DApplication::OnQuerySuspend(DWORD dwFlags)  { (void)dwFlags; return 1; }
LRESULT CD3DApplication::OnResumeSuspend(DWORD dwData)  { (void)dwData; return 1; }

VOID CD3DApplication::Cleanup3DEnvironment()
{
    m_bActive = FALSE;
    m_bReady  = FALSE;
    DeleteDeviceObjects();
}

VOID CD3DApplication::DeleteDeviceObjects()
{
    if (m_pD3DEngine != NULL)
    {
        m_pD3DEngine->DeleteDeviceObjects();
    }
}

void CD3DApplication::InitText()  { }
void CD3DApplication::DrawSuppl() { }
VOID CD3DApplication::ShowStats() { }
VOID CD3DApplication::OutputText(DWORD x, DWORD y, TCHAR* str) { (void)x; (void)y; (void)str; }
VOID CD3DApplication::DisplayFrameworkError(HRESULT hr, DWORD dwType)
{
    SDL_Log("Framework error hr=%d type=%u", (int)hr, (unsigned)dwType);
}

// ---------------------------------------------------------------------------
// keys
// ---------------------------------------------------------------------------

void CD3DApplication::FlushPressKey()
{
    m_keyState = 0;
    m_axeKeyX = 0.0f;
    m_axeKeyY = 0.0f;
    m_axeKeyZ = 0.0f;
    m_axeKeyW = 0.0f;
    m_axeJoy = D3DVECTOR(0.0f, 0.0f, 0.0f);
}

void CD3DApplication::ResetKey()
{
    int i;
    for (i = 0; i < 50; i++)
    {
        m_key[i][0] = 0;
        m_key[i][1] = 0;
    }
    m_key[KEYRANK_LEFT   ][0] = VK_LEFT;
    m_key[KEYRANK_RIGHT  ][0] = VK_RIGHT;
    m_key[KEYRANK_UP     ][0] = VK_UP;
    m_key[KEYRANK_DOWN   ][0] = VK_DOWN;
    m_key[KEYRANK_ROTCW  ][0] = VK_WHEELUP;
    m_key[KEYRANK_ROTCW  ][1] = VK_DELETE;
    m_key[KEYRANK_ROTCCW ][0] = VK_WHEELDOWN;
    m_key[KEYRANK_ROTCCW ][1] = VK_NEXT;
    m_key[KEYRANK_STOP   ][0] = VK_SPACE;
    m_key[KEYRANK_HELP   ][0] = VK_F1;
    m_key[KEYRANK_NEAR   ][0] = VK_ADD;
    m_key[KEYRANK_AWAY   ][0] = VK_SUBTRACT;
    m_key[KEYRANK_QUIT   ][0] = VK_ESCAPE;
    m_key[KEYRANK_SPEED10][0] = VK_F4;
    m_key[KEYRANK_SPEED15][0] = VK_F5;
    m_key[KEYRANK_SPEED20][0] = VK_F6;
}

void CD3DApplication::SetKey(int keyRank, int option, int key)
{
    if (keyRank < 0 || keyRank >= 50) return;
    if (option < 0 || option >= 2) return;
    m_key[keyRank][option] = key;
}

int CD3DApplication::RetKey(int keyRank, int option)
{
    if (keyRank < 0 || keyRank >= 50) return 0;
    if (option < 0 || option >= 2) return 0;
    return (int)m_key[keyRank][option];
}

BOOL CD3DApplication::IsKeyMouse(int key)
{
    return (key == VK_WHEELUP || key == VK_WHEELDOWN);
}

BOOL CD3DApplication::IsKeyJoystick(int key)
{
    return (key >= VK_BUTTON1 && key <= VK_BUTTON32);
}

void CD3DApplication::SetForce(float force) { m_FFBforce = force; }
float CD3DApplication::RetForce()           { return m_FFBforce; }
void CD3DApplication::SetFFB(BOOL bMode)    { m_bFFB = bMode; }
BOOL CD3DApplication::RetFFB()              { return m_bFFB; }

void CD3DApplication::SetJoystick(int mode)
{
    m_joystick = mode;
    if (m_joystick != 0)
    {
        if (!InitDirectInput(m_instance, m_hWnd, m_bFFB))
        {
            m_joystick = 0;
        }
        else
        {
            SetAcquire(TRUE);
        }
    }
    else
    {
        SetAcquire(FALSE);
        FreeDirectInput();
    }
}

int CD3DApplication::RetJoystick() { return m_joystick; }

BOOL CD3DApplication::SetJoyForces(float forceX, float forceY)
{
    (void)forceX; (void)forceY;
    return TRUE;
}

// ---------------------------------------------------------------------------
// display settings (single GL "device")
// ---------------------------------------------------------------------------

BOOL CD3DApplication::EnumDevices(char *bufDevices, int lenDevices,
                                  char *bufModes, int lenModes,
                                  int &totalDevices, int &selectDevices,
                                  int &totalModes, int &selectModes)
{
    (void)lenDevices; (void)lenModes;
    strcpy(bufDevices, "OpenGL");
    bufDevices[strlen(bufDevices)+1] = 0;   // double-null terminated list

    sprintf(bufModes, "%dx%dx32", g_winWidth, g_winHeight);
    bufModes[strlen(bufModes)+1] = 0;

    totalDevices = 1;
    selectDevices = 0;
    totalModes = 1;
    selectModes = 0;
    return TRUE;
}

BOOL CD3DApplication::RetFullScreen()
{
    if (g_window == NULL) return FALSE;
    Uint32 flags = SDL_GetWindowFlags(g_window);
    return (flags & (SDL_WINDOW_FULLSCREEN|SDL_WINDOW_FULLSCREEN_DESKTOP)) != 0;
}

BOOL CD3DApplication::ChangeDevice(char *device, char *mode, BOOL bFull)
{
    (void)device; (void)mode;
#ifndef __ANDROID__
    SDL_SetWindowFullscreen(g_window, bFull ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    SDL_GL_GetDrawableSize(g_window, &g_winWidth, &g_winHeight);
    if (m_pD3DDevice != NULL) m_pD3DDevice->Resize(g_winWidth, g_winHeight);
    m_ddsdRenderTarget.dwWidth  = (DWORD)g_winWidth;
    m_ddsdRenderTarget.dwHeight = (DWORD)g_winHeight;
#else
    (void)bFull;
#endif
    return TRUE;
}

// screenshot: read back the framebuffer and write a 24-bit BMP
BOOL CD3DApplication::WriteScreenShot(char *filename, int width, int height)
{
    (void)width; (void)height;
    int w = g_winWidth, h = g_winHeight;
    unsigned char* pixels = (unsigned char*)malloc((size_t)w*h*4);
    if (pixels == NULL) return FALSE;
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    FILE* f = fopen(filename, "wb");
    if (f == NULL) { free(pixels); return FALSE; }

    int rowSize = (w*3 + 3) & ~3;
    int imgSize = rowSize * h;
    unsigned char hdr[54];
    memset(hdr, 0, sizeof(hdr));
    hdr[0]='B'; hdr[1]='M';
    *(int*)(hdr+2)  = 54 + imgSize;
    *(int*)(hdr+10) = 54;
    *(int*)(hdr+14) = 40;
    *(int*)(hdr+18) = w;
    *(int*)(hdr+22) = h;
    *(short*)(hdr+26) = 1;
    *(short*)(hdr+28) = 24;
    *(int*)(hdr+34) = imgSize;
    fwrite(hdr, 1, 54, f);

    unsigned char* row = (unsigned char*)calloc(1, (size_t)rowSize);
    for (int y = 0; y < h; y++)     // GL rows are already bottom-up like BMP
    {
        const unsigned char* src = pixels + (size_t)y*w*4;
        for (int x = 0; x < w; x++)
        {
            row[x*3+0] = src[x*4+2];
            row[x*3+1] = src[x*4+1];
            row[x*3+2] = src[x*4+0];
        }
        fwrite(row, 1, rowSize, f);
    }
    free(row);
    fclose(f);
    free(pixels);
    return TRUE;
}

// ---------------------------------------------------------------------------
// entry point (SDL_main on Android)
// ---------------------------------------------------------------------------

static void PortLogToStderr(void* userdata, int category, SDL_LogPriority priority,
                            const char* message)
{
    (void)userdata; (void)category; (void)priority;
    fprintf(stderr, "%s\n", message);
    fflush(stderr);
}

#ifdef __ANDROID__
extern "C" void PortAndroidBootstrap(void);
#endif

extern "C" int main(int argc, char* argv[])
{
#ifndef __ANDROID__
    SDL_LogSetOutputFunction(PortLogToStderr, NULL);
#endif
    fprintf(stderr, "[port] main start\n");
    fflush(stderr);

#ifdef __ANDROID__
    // extract game data from the APK on first launch, chdir to it
    PortAndroidBootstrap();
#endif

    char cmdLine[256];
    cmdLine[0] = 0;
    for (int i = 1; i < argc && strlen(cmdLine)+strlen(argv[i])+2 < sizeof(cmdLine); i++)
    {
        strcat(cmdLine, argv[i]);
        strcat(cmdLine, " ");
    }

    CD3DApplication d3dApp;

    Error err = d3dApp.CheckMistery(cmdLine);
    if (err != ERR_OK)
    {
        char string[100];
        GetResource(RES_ERR, err, string);
        SDL_Log("Startup error: %s", string);
        return 0;
    }

    fprintf(stderr, "[port] creating application\n");
    fflush(stderr);
    if (FAILED(d3dApp.Create(NULL, cmdLine)))
    {
        fprintf(stderr, "[port] Create() failed\n");
        fflush(stderr);
        return 0;
    }

    fprintf(stderr, "[port] entering main loop\n");
    fflush(stderr);
    return d3dApp.Run();
}
