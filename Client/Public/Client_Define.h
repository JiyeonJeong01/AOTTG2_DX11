#pragma once

#include <Windows.h>
#include "Engine_SDK.h"
#include "Engine_Component.h"
#include "Engine_Math.h"
#include "GameInstance.h"
#include "magic_enum.hpp"

extern HWND g_hWnd;
extern HINSTANCE g_hInst;

namespace Client
{
    static unsigned int		g_iWinSizeX = 1920;
    static unsigned int		g_iWinSizeY = 1080;

    static unsigned int     g_SceneX = 1920;
    static unsigned int     g_SceneY = 1080;

    enum class SIDE {
        NONE = 0,
        LEFT = 1 << 0, // 1
        RIGHT = 1 << 1, // 2
        BOTH = To<_uint>(LEFT) | To<_uint>(RIGHT) // 3
    };

    enum OBJECT_MASK
    {
        HUMAN       = 1 << 0,
        TITAN       = 1 << 1,
        PLAYER      = 1 << 2,
        NPC         = 1 << 3,
        ALLY        = 1 << 4, 
        WALKABLE    = 1 << 5,

    };

    typedef struct tagDisplacement
    {
        _float3 vDir{};
        _float3 vDirXZ{};
        _float  fDist{};

        tagDisplacement(_float3 vInput)
        {
            _vector vVec = XMLoadFloat3(&vInput);
            _vector vLen = XMVector3Length(vVec);
            fDist = XMVectorGetX(vLen);
            XMStoreFloat3(&vDir, XMVector3Normalize(vVec));

            _vector vMove = XMVectorSetY(vVec, 0.f);
            if (XMVectorGetX(XMVector3LengthSq(vMove)) > 0.f)
                XMStoreFloat3(&vDirXZ, XMVector3Normalize(vMove));
            else
                vDirXZ = { 0.f, 0.f, 0.f };
        }
    } DISPLACEMENT;

    typedef struct tagEntityVolume
    {
        _float3 vMin{};
        _float3 vMax{};
    } ENTITY_VOLUME;


}



using namespace Client;
