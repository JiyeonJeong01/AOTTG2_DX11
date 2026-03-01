#include "Raycast.h"

IMPLEMENT_SINGLETON(CRaycast)

CRaycast::CRaycast()
{
    
}
CRaycast::~CRaycast()
{
    
}


bool CRaycast::Intersect_Ray(RAYCAST_HIT* tOut, const RAY& ray)
{
    return {};
}

RAY CRaycast::Build_Ray_From_Screen(float mouseX, float mouseY, float vpW, float vpH, const _float4x4& matView,
    const _float4x4& matProj)
{
    return {};
}
