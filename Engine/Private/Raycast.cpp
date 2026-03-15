#include "Raycast.h"
#include "CRender_System.h"
#include "Engine_Math.h"
#include "Render_Context.h"
#include "Physics_Processor.h"

_bool CRaycast::Intersect_Ray(RAYCAST_HITS& outHits, RAY& tRAY, const RAY& ray)
{
    return {};
}


_bool CRaycast::Intersect_Ray(RAYCAST_HITS& outHits, RAY& tRAY, const POINT& ptGame, CPhysics_Processor* pPhysics)
{
    if (!pPhysics || ptGame.x < 0 || ptGame.y < 0)
        return false;

    UI_GLOBAL tUI = SYS_RENDER.Contexts()->Get_UI_Global();
    _matrix matView = Math::Load(SYS_RENDER.Contexts()->Get_View());
    _matrix matProj = Math::Load(SYS_RENDER.Contexts()->Get_Proj());

    const _float fGameX = To<_float>(ptGame.x);
    const _float fGameY = To<_float>(ptGame.y);

    _vector vNearScreen = { fGameX, fGameY, 0.f, 1.f };
    _vector vFarScreen = { fGameX, fGameY, 1.f, 1.f };

    const _vector vNearWorld = XMVector3Unproject(
        vNearScreen,
        0.f, 0.f,
        tUI.vViewport.x, tUI.vViewport.y,
        0.f,        1.f,
        matProj, matView, XMMatrixIdentity());

    const _vector vFarWorld = XMVector3Unproject(
        vFarScreen,
        0.f, 0.f,
        tUI.vViewport.x, tUI.vViewport.y,
        0.f, 1.f,
        matProj, matView, XMMatrixIdentity());

    _vector vRayDir = Math::Normalize(vFarWorld - vNearWorld);
    _float3 vOriginF{}, vRayDirF{};
    Math::Store(tRAY.vOrigin, vNearWorld);
    Math::Store(tRAY.vDir, vRayDir);

    pPhysics->Detect_Raycast(tRAY, outHits);

    return false;
}

RAY CRaycast::Build_Ray_From_Screen(float mouseX, float mouseY, float vpW, float vpH, const _float4x4& matView,
                                    const _float4x4& matProj)
{
    return {};
}
