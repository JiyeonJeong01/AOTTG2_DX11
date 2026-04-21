// Editor_System.h
#pragma once
#include "Base.h"

#include "BuiltIn_GUID.h"
#include "Event.h"
#include "GameObject_Event.h"
#include "Nav_Struct.h"
#include "Debug_Renderer.h"

NS_BEGIN(Engine)
    class CTransform_Processor;
class CGameObject;

class ENGINE_DLL CEditor_System final
{
    DECLARE_SINGLETON(CEditor_System)

public:
    HRESULT Initialize(const std::filesystem::path& assetRoot, ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    void    Update(_float fDT);
    void    Render();

/* -------------------- Scene -------------------- */
public:
    ASSET_GUID Ensure_DefaultScene();
    ASSET_GUID Create_NewScene_Asset(std::filesystem::path* outPath = nullptr);

    void        On_SceneChanged(EVENT_DATA& event);

    void        Play();
    void        Pause();
    void        Step(_float fDT);

/* ---------------- SceneView(Debug) Camera ---------------- */
public:
    void Update_SceneView_State(_float fWidth, _float fHeight);

    void Submit_DebugCamera();
    void Toggle_DebugCamera(_bool bToggle);

    void Focus_Object(CGameObject* pObj);

    _float3 Get_Position() const;
    _float3 Get_RotationEuler() const;


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
    class CScene*           m_pCurScene{};

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
    _float  m_fWheelZoomSpeed = 1.5f;

/* -------------------- SceneView camera -------------------- */
private :
    CTransform_Processor*           m_pTransform_Processor{};
    OBJECT_HANDLE                   m_hSelectedObject{};
    CEvent<GAMEOBJECT_EVENT_DATA&>  m_OnPicking{};
    _bool                           m_bDebugCam = true;


/* -------------------- Nav Edit -------------------- */
private:
    std::vector<NAV_POINT> m_vecNavPoints;                              // 전체 Nav 점 목록
    std::vector<NAV_CELL>  m_vecNavCells;                               // 삼각형 셀 목록

    std::vector<NAV_POINT> m_vecSavedNavPoints;                         // 저장된 Nav 점 목록
    std::vector<NAV_CELL>  m_vecSavedNavCells;                          // 저장된 Nav 셀 목록

    _int                   m_iPickedPointIndices[3] = { -1, -1, -1 };   // 지금 찍는 중인 점 세 개의 인덱스 임시 저장
    _int                   m_iPickedPointCount = 0;                     // 현재 몇 개 찍었는지

    _float                 m_fNavCellY = 0.f;
    _float                 m_fPointSnapRange = 1.f;                    // 이 범위 내면 같은 점으로 판단

    std::vector<class CNavMesh*>    m_allNavDebug;
public:
    _bool   Pick_Cell(_uint px, _uint py, _uint vpW, _uint vpH, _float3& vOutPoint);
    _int    Find_Or_Add_NavPoint(const _float3& vPoint);
    void    Add_CellPoint(const _float3& vPoint);
    void    Clear_PickedCellPoints();
    void    Render_NavCells(CDebug_Renderer* pDebugRenderer);
    void    Save_Nav();
    _bool   Load_SavedNav(const wchar_t* pFilePath);
    void    Add_NavDebugRenderer(class CNavMesh*);
};

NS_END
