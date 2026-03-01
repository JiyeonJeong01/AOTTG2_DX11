#include "Engine_Define.h"

struct RAY
{
    _float3 vOrigin{};
    _float3 vDir{};   /* normalize */ 
};

struct RAYCAST_HIT
{
    OBJECT_HANDLE hObject{};
    _float t = FLT_MAX;
    _float3  vPos{};
    _float3  vNormal{};
};

class ENGINE_DLL CRaycast final
{
    DECLARE_SINGLETON(CRaycast)

public:
    bool Intersect_Ray(RAYCAST_HIT* tOut, const RAY& ray);
    RAY Build_Ray_From_Screen(float mouseX, float mouseY, float vpW, float vpH,
        const _float4x4& matView, const _float4x4& matProj);
};
