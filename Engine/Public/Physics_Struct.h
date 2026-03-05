#pragma once

#include "Engine_Define.h"

NS_BEGIN(Engine)

typedef struct tagAABB
{
    _float3 vMin{ 0.f,0.f,0.f };
    _float3 vMax{ 0.f,0.f,0.f };
    _bool   bValid{ true };
}AABB;



NS_END
