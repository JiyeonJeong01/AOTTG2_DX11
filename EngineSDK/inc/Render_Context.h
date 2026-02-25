// Render_Context.h
#pragma once

#include "Engine_Define.h"

NS_BEGIN(Engine)

class CRender_Context final
{
public:
    CRender_Context();
    ~CRender_Context();

public:
    HRESULT Initialize(_float fWidth, _float fHeight);

public:
    // Read-only access
    const _float4x4& Get_View() const;
    const _float4x4& Get_Proj() const;
    const _float4x4& Get_ViewInv() const;
    const _float4x4& Get_ProjInv() const;

    const _float4x4& Get_ViewProj() const;
    const _float4x4& Get_ViewProjInv() const;

    const _float3& Get_CamPosition() const;

    const UI_GLOBAL& Get_UI_Global();
    void Set_UI_Global(const UI_GLOBAL& tUI);


public:
    // Write (Render_System / Camera submission only)
    void Set_View(_fmatrix view);
    void Set_Proj(_fmatrix proj);
    void Set_ViewProj(_fmatrix view, _fmatrix proj);

    // Call once per frame after setting view/proj
    void Update();

private:
    void Update_Inverses_IfNeeded();
    void Update_ViewProj_IfNeeded();
    void Update_CamPos_From_ViewInv();

    void On_Resize(EVENT_DATA& eData);


private:
    _float4x4 m_matView{};
    _float4x4 m_matProj{};
    _float4x4 m_matViewInv{};
    _float4x4 m_matProjInv{};

    _float4x4 m_matViewProj{};
    _float4x4 m_matViewProjInv{};

    _float3   m_vCamPos{ 0.f, 0.f, 0.f };

    _bool m_bDirtyInvView = true;
    _bool m_bDirtyInvProj = true;
    _bool m_bDirtyViewProj = true;

    UI_GLOBAL   m_gUI{};


public:
    static std::unique_ptr<CRender_Context> Create(_uint iWidth, _uint iHeight);
};

NS_END
