// Render_Context.cpp
#include "Render_Context.h"

#include "Engine_Log.h"
#include "Engine_Math.h"
#include "Event_System.h"
#include "WindowResize_Event.h"

NS_BEGIN(Engine)

CRender_Context::CRender_Context() = default;
CRender_Context::~CRender_Context() = default;

HRESULT CRender_Context::Initialize(_float fWidth, _float fHeight)
{
    SYS_EVENT.Subscribe(EVENT_TYPE::On_Window_Resize, &CRender_Context::On_Resize, this);

    const _vector vEye = XMVectorSet(0.f, 5.f, -5.f, 0.f);
    const _vector vAt = XMVectorSet(0.f, 0.f, 0.f, 0.f);
    const _vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
    const _matrix matView = XMMatrixLookAtLH(vEye, vAt, vUp);

    const _matrix matProj = XMMatrixPerspectiveFovLH(XM_PIDIV2, 16.0f / 9.0f, 0.01f, 1000.0f);
    XMStoreFloat4x4(&m_matView, matView);
    XMStoreFloat4x4(&m_matProj, matProj);

    m_gUI.vViewport = { SCAST(_float, fWidth), SCAST(_float, fHeight) };
    Math::Store(m_gUI.matView, XMMatrixIdentity());
    Math::Store(m_gUI.matProj, XMMatrixOrthographicOffCenterLH(0.f, fWidth, fHeight, 0.f, 0.f, 1.f));

    return S_OK;
}

const _float4x4& CRender_Context::Get_View() const
{
    return m_matView;
}
const _float4x4& CRender_Context::Get_Proj() const
{
    return m_matProj;
}
const _float4x4& CRender_Context::Get_ViewInv() const
{
    return m_matViewInv;
}
const _float4x4& CRender_Context::Get_ProjInv() const
{
    return m_matProjInv;
}

const _float4x4& CRender_Context::Get_ViewProj() const
{
    return m_matViewProj;
}
const _float4x4& CRender_Context::Get_ViewProjInv() const
{
    return m_matViewProjInv;
}

const _float3& CRender_Context::Get_CamPosition() const
{
    return m_vCamPos;
}

const UI_GLOBAL& CRender_Context::Get_UI_Global()
{
    return m_gUI;
}

void CRender_Context::Set_UI_Global(const UI_GLOBAL& tUI)
{
    __debugbreak(); // 누가 덮는지 콜스택으로 잡기

    m_gUI = tUI;
}

void CRender_Context::Set_View(_fmatrix view)
{
    XMStoreFloat4x4(&m_matView, view);
    m_bDirtyInvView = true;
    m_bDirtyViewProj = true;
}

void CRender_Context::Set_Proj(_fmatrix proj)
{
    XMStoreFloat4x4(&m_matProj, proj);
    m_bDirtyInvProj = true;
    m_bDirtyViewProj = true;
}

void CRender_Context::Set_ViewProj(_fmatrix view, _fmatrix proj)
{
    XMStoreFloat4x4(&m_matView, view);
    XMStoreFloat4x4(&m_matProj, proj);
    m_bDirtyInvView = true;
    m_bDirtyInvProj = true;
    m_bDirtyViewProj = true;
}

void CRender_Context::Update()
{
    Update_Inverses_IfNeeded();
    Update_ViewProj_IfNeeded();
    Update_CamPos_From_ViewInv();
}

void CRender_Context::Update_Inverses_IfNeeded()
{
    if (m_bDirtyInvView)
    {
        XMStoreFloat4x4(&m_matViewInv,
            XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_matView)));
        m_bDirtyInvView = false;
    }

    if (m_bDirtyInvProj)
    {
        XMStoreFloat4x4(&m_matProjInv,
            XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_matProj)));
        m_bDirtyInvProj = false;
    }
}

void CRender_Context::Update_ViewProj_IfNeeded()
{
    if (!m_bDirtyViewProj)
        return;

    const XMMATRIX view = XMLoadFloat4x4(&m_matView);
    const XMMATRIX proj = XMLoadFloat4x4(&m_matProj);
    const XMMATRIX vp = XMMatrixMultiply(view, proj);

    XMStoreFloat4x4(&m_matViewProj, vp);
    XMStoreFloat4x4(&m_matViewProjInv, XMMatrixInverse(nullptr, vp));

    m_bDirtyViewProj = false;
}

void CRender_Context::Update_CamPos_From_ViewInv()
{
    // NOTE:
    // This assumes translation sits in m[3][0..2] of XMFLOAT4X4 for your convention.
    // If your math convention differs, adjust this extraction accordingly.
    m_vCamPos = { m_matViewInv.m[3][0], m_matViewInv.m[3][1], m_matViewInv.m[3][2] };
}

std::unique_ptr<CRender_Context> CRender_Context::Create(_uint iWidth, _uint iHeight)
{
    auto pInstance = std::make_unique<CRender_Context>();
    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize((_float)iWidth, (_float)iHeight), nullptr, "Instance create failed");
    return pInstance;
}

void CRender_Context::On_Resize(EVENT_DATA& eData)
{
    __debugbreak(); // 누가 덮는지 콜스택으로 잡기

    assert(eData.eType == EVENT_TYPE::On_Window_Resize);

    auto& eResizeData = SCAST(RESIZE_EVENT_DATA&, eData);

    m_gUI.vViewport = { SCAST(_float, eResizeData.iWidth), SCAST(_float, eResizeData.iHeight) };

    Math::Store(m_gUI.matProj, XMMatrixOrthographicOffCenterLH(
        0.f, SCAST(_float, eResizeData.iWidth),
        SCAST(_float, eResizeData.iHeight), 0.f,
        0.f, 1.f));
}


NS_END
