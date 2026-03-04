// Editor_System.h
#pragma once
#include "Base.h"

#include "BuiltIn_GUID.h"
#include "Event.h"
#include "GameObject_Event.h"

NS_BEGIN(Engine)
class CTransform_Processor;
class CGameObject;

class ENGINE_DLL CEditor_System final
{
    DECLARE_SINGLETON(CEditor_System)

public:
    HRESULT Initialize(const std::filesystem::path& assetRoot);
        void    Update(_float fDT);

/* -------------------- Scene -------------------- */
public:
    ASSET_GUID Ensure_DefaultScene();
    ASSET_GUID Create_NewScene_Asset(std::filesystem::path* outPath = nullptr);

    void        On_SceneChanged(EVENT_DATA& event);

    void        Play();
    void        Pause();
    void        Step(_float fDT);

/* ---------------- SceneView Camera ---------------- */
public:  
    void Submit_SceneViewCamera();
    void Update_SceneView_State(_float fWidth, _float fHeight);
    void Toggle_SceneViewCamera(_bool bToggle);
    void Apply_SceneView_From_View16(const float* view16);

private: /* SceneView Camera */
    void Build_SceneView_Matrices();
    void Update_Input(_float fDT);

/* -------------------- 마우스 피킹 ------------------- */
public :
    void Pick_SceneView(_uint px, _uint py, _uint vpW, _uint vpH);
    template<typename T>
    ListenerID Subscribe(void(T::* func)(GAMEOBJECT_EVENT_DATA&), T* obj)
    {
        return m_OnPicking.Add_Listener(func, obj);
    }

/* -------------------- Scene -------------------- */
private:
    std::filesystem::path   m_pathAsset{};
    class CScene* m_pCurScene{};

/* -------------------- SceneView camera -------------------- */
private:
    _float4x4   m_matCamView{};
    _float4x4   m_matCamProj{};

    _float3     m_vCamPos{ 0.f, 2.f, -5.f };
    _float      m_fCamYaw = 0.f, m_fYaw = 0.f;
    _float      m_fCamPitch = 0.f, m_fPitch = 0.f;

    _float      m_fFovyRad = 1.0471976f; /* 60 degree -> radian */
    _float      m_fAspect = 0.f;
    _float      m_fNear = 0.1f;
    _float      m_fFar = 1000.f;

    /* 이동 입력/가속 관련 */ 
    _float   m_fCamAcc = 1.f, m_fCamSpeed = 3.f;
    _float   m_fCamDamping = 10.f;
    _float3  m_vCamVel = { 0.f, 0.f, 0.f };

    /* 회전 입력 관련 */
    _bool    m_bCalculAcc = true; 

    _float  m_fMouseSens = 3.f;
    _float  m_fWheelZoomSpeed = 3.f;
/* -------------------- SceneView camera -------------------- */
private :
    CTransform_Processor*           m_pTransform_Processor{};
    OBJECT_HANDLE                   m_hSelectedObject{};
    CEvent<GAMEOBJECT_EVENT_DATA&>  m_OnPicking{};
    _bool                           m_bScencViewCam = false;
};

NS_END
