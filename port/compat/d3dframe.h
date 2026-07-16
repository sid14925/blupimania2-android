// D3DFrame.h — shim replacing the DX7 SDK framework class. The SDL platform
// layer (port/sdl/sdlapp.cpp) owns the window and GL context; this class is
// only kept so that d3dapp.h compiles unchanged.

#ifndef _PORT_D3DFRAME_H_
#define _PORT_D3DFRAME_H_

#include <windows.h>
#include <ddraw.h>
#include <d3d.h>

class CD3DFramework7
{
public:
    CD3DFramework7()  { }
    ~CD3DFramework7() { }
};

#endif // _PORT_D3DFRAME_H_
