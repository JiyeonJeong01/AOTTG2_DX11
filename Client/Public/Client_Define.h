#pragma once

#include <Windows.h>
#include "Engine_SDK.h"
#include "Engine_Component.h"
#include "Engine_Math.h"
#include "GameInstance.h"
#include "magic_enum.hpp"
#include "Input_System.h"

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
        O_NONE = 0,

        O_PLAYER = 1 << 0,
        O_SCOUT = 1 << 1,
        O_EREN = 1 << 2,

        O_ENEMY = 1 << 3,

        O_HITBOX = 1 << 4,
        O_HURTBOX = 1 << 5,

        O_WALKABLE = 1 << 6,

        O_ETC1 = 1 << 7,
        O_ETC2 = 1 << 8,
        O_ETC3 = 1 << 9,
        O_ETC4 = 1 << 10,
        O_ETC5 = 1 << 11
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

    typedef struct tagHitInfo
    {
        CGameObject*    goAttacker{};
        _float          fDamage{};
        _float3         vHitPoint{};

    } HIT_INFO;

    typedef struct tagPatrolInfo
    {
        _float3 vPos[2] = {};

        _uint   iPatrolIndex = 0;
        _float3 Get_CurPatrolPos()
        {
            if (iPatrolIndex >= 2) iPatrolIndex %= 2;

            return vPos[iPatrolIndex];
        }
        void Update_PatrolPos()
        {
            iPatrolIndex++;
            if (iPatrolIndex >= 2)
                iPatrolIndex %= 2;
        }
    } PATROL_INFO;


}



using namespace Client;
