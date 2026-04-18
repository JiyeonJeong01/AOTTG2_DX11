#pragma once
#include "Engine_Define.h"
#include "Component_Struct.h"
#include "Identity.h"
#include "Component_System.h"

#pragma region FOWARD DECLARATION
NS_BEGIN(Engine)
class CGameObject;
class CPhysics_Processor;
class CLine;
class CTransform;
class CNavMesh;
typedef struct tagRay RAY;
typedef struct tagRaycastHit RAYCAST_HIT;
typedef struct tagRaycastHits RAYCAST_HITS;
typedef struct tagAnimatorData ANIMATOR_DATA;
typedef struct tagPostProcessDesc POST_PROCESS_DESC;
NS_END
#pragma endregion

NS_BEGIN(Engine)

class ENGINE_DLL CGameInstance final
{
    DECLARE_SINGLETON(CGameInstance)

public:
    HRESULT Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    HRESULT SetUp_Game();

public: /* ----------- Scene ----------- */
    HRESULT Change_Scene(const std::string& strScene);

public: /* -------- GameObject --------- */
    CGameObject* Find_GameObject(OBJECT_HANDLE hObj);
    CGameObject* Find_GameObject(const std::string& strName);

    CGameObject* Instantiate(const string& strProto,
        Layer::LAYER_ID iLayer = Layer::DEFAULT_LAYER,
        const string& strName = "GameObject_Clone",
        CGameObject* pParent = nullptr);
    CGameObject* Instantiate(const ASSET_GUID& tGUID,
        Layer::LAYER_ID iLayer = Layer::DEFAULT_LAYER,
        const string& strName = "GameObject_Clone",
        CGameObject* pParent = nullptr);

public : /* -------- Raycast -------- */
    _bool   Raycast(const POINT& pt, RAY& tRAY, RAYCAST_HIT& tHitInfo);
    _bool   RaycastAll(const POINT& pt, RAY& tRAY, RAYCAST_HITS& tAllHitInfo);

public : /* ---------- Game ---------- */
    _float      Get_DT() const noexcept;
    void        Set_TimeScale();
    void        Pause();
    void        Play();

public : /* ---------- Camera ---------- */
    _float3                 Cam_Position() const;
    _float3                 Cam_Look() const;
    const _float4x4&        Get_View()  const;
    const _float4x4&        Get_Proj()  const;
    const UI_GLOBAL&        Get_UI_Global() const;
    const D3D11_VIEWPORT&   Get_Viewport() const;

public : /* ---------- Animation ---------- */
    _bool Find_AttachBoneInfo(OBJECT_HANDLE hTargetObj, const string& strTargetBoneName, ANIMATOR_DATA*& pOutAnimator, _uint& iOutBoneIndex);
    void  Calculate_AttachBoneMatrixPtr(const CTransform& hTargetTrans, CTransform& hAttachTrans, const _float4x4* matCombinedPtr);

public :/* ---------- Built-in ---------- */
    unique_ptr<CLine>   Load_LineMesh(_uint iNumPoint, _float fThickness, LINE_TYPE eType);

    uint32_t                    Get_ResourceHandle(ASSET_TYPE eType, const ASSET_GUID& tGUID);
    uint32_t                    Alloc_PerObjectParamBlock();
    PER_OBJECT_PARAM_BLOCK*     Get_PerObjectParamBlock(uint32_t handle);

    std::unique_ptr<CNavMesh> Create_NavMesh(const wchar_t* pFilePath, _bool bDebugRender = false);

private :
    ID3D11Device*               m_pDevice = nullptr;
    ID3D11DeviceContext*        m_pContext = nullptr;

    GameConfig::GAME_CONFIG     m_tGameConfig{};

    CPhysics_Processor*         m_pPhysics{};

private : /* ----------- Scene ----------- */
    _bool Read_GameConfig();
    _bool Resolve_SceneGUID(const std::string& strScene, ASSET_GUID& outGUID);

public : /* ---------- Post Process ----------- */
    void Set_PostProcessDesc(const POST_PROCESS_DESC& tPostProcessDesc);
};


NS_END
