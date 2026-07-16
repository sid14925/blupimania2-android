// D3DEnum.h — shim replacing DX7 device enumeration. The SDL/GL port exposes
// exactly one "device" (the GL context); d3dapp.h only holds a pointer.

#ifndef _PORT_D3DENUM_H_
#define _PORT_D3DENUM_H_

#include <windows.h>
#include <ddraw.h>
#include <d3d.h>

struct D3DEnum_DeviceInfo
{
    char strDesc[40];
    BOOL bHardware;
    BOOL bWindowed;
    GUID guidDevice;
};

#endif // _PORT_D3DENUM_H_
