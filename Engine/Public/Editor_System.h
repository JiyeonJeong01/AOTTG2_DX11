// Editor_System.h
#pragma once
#include "Base.h"

#include "BuiltIn_GUID.h"
#include "Engine_Math.h" // 추가

NS_BEGIN(Engine)

class ENGINE_DLL CEditor_System final
{
    DECLARE_SINGLETON(CEditor_System)

public:
    HRESULT Initialize(const std::filesystem::path& assetRoot); \
        void    Update(_float fDT);

public:
    ASSET_GUID Ensure_DefaultScene();
    ASSET_GUID Create_NewScene_Asset(std::filesystem::path* outPath = nullptr);

    void        On_SceneChanged(EVENT_DATA& event);

    void        Play();
    void        Pause();
    void        Step(_float fDT);

public:
    void Submit_SceneViewCamera();
    void Update_SceneView_State(_float fWidth, _float fHeight);

private: /* SceneView Camera */
    void Build_SceneView_Matrices();
    void Update_Input(_float fDT);

private:
    std::filesystem::path   m_pathAsset{};
    class CScene* m_pCurScene{};

private:
    // ----- Editor free camera state (minimal) -----
    _float4x4   m_matCamView{};
    _float4x4   m_matCamProj{};

    _float3     m_vCamPos{ 0.f, 2.f, -5.f };
    _float      m_fCamYaw = 0.f, m_fYaw = 0.f;
    _float      m_fCamPitch = 0.f, m_fPitch = 0.f;

    _float      m_fFovyRad = 1.0471976f; /* 60 degree -> radian */
    _float      m_fAspect = 0.f;
    _float      m_fNear = 0.1f;
    _float      m_fFar = 1000.f;

    // 이동 입력/가속 관련
    _float   m_fCamAcc = 1.f, m_fCamSpeed = 3.f;     // 가속도(초당 속도 변화량)
    _float   m_fCamDamping = 10.f; // 감속(클수록 빨리 멈춤)
    _float3  m_vCamVel = { 0.f, 0.f, 0.f }; // 현재 카메라 속도

    // 회전 입력 관련
    _bool    m_bRMBDown = false, m_bCalculAcc = true;   // 우클릭 드래그 회전용 (원하시면 조건 변경 가능)

    _float m_fMouseSens = 3.f;

};

NS_END
