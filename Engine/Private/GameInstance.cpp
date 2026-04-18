#include "GameInstance.h"

#include "Core_System.h"
#include "GameObject_System.h"
#include "Component_System.h"
#include "Input_System.h"
#include "Logger.h"
#include "CRender_System.h"
#include "Render_Context.h"
#include "Editor_System.h"

#include "Asset_Registry.h"
#include "Resource_System.h"

#include "Engine_Log.h"
#include "GameObject.h"

#include "Raycast.h"
#include "Physics_Processor.h"

#include "Animator_Processor.h"
#include "MeshRenderer_Processor.h"

#include "Mesh.h"
#include "MeshBuilder.h"
#include "Line.h"

#include "NavMesh.h"

IMPLEMENT_SINGLETON(CGameInstance)

CGameInstance::~CGameInstance()
{
}

CGameInstance::CGameInstance()
{
}

unique_ptr<CLine> s_Line;

HRESULT CGameInstance::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    m_pDevice = pDevice;
    m_pContext = pContext;

    Read_GameConfig();

    m_pPhysics = SYS_COMPONENT.Bind_Processor<CPhysics_Processor>();
    IF_NULL_RETURN_MSG_BREAK(m_pPhysics, E_FAIL, "CPhysics Processor is nullptr.");
    return S_OK;
}

HRESULT CGameInstance::SetUp_Game()
{
    std::string strScene;

    if (!m_tGameConfig.startSceneName.empty())
        strScene = m_tGameConfig.startSceneName;
    else
        strScene = "Untitled.scene"; /* 기본 씬 */

    ASSET_GUID outGUID{};
    /* 시작 씬 이름 -> GUID */
    IF_TRUE_RETURN_MSG_BREAK(!Resolve_SceneGUID(strScene, outGUID), E_FAIL, "SetUp_Game failed: Resolve_SceneGUID failed");

    /* GUID -> 씬 시작 */
    IF_FAIL_RETURN_MSG_BREAK(SYS_CORE.Change_Scene(outGUID), E_FAIL, "SetUp_Game failed: Change_Scene failed");

    return S_OK;
}

HRESULT CGameInstance::Change_Scene(const std::string& strScene)
{
    return SYS_CORE.Change_Scene(strScene);
}

CGameObject* CGameInstance::Find_GameObject(OBJECT_HANDLE hObj)
{
    return SYS_GAMEOBJECT.Get_Wrapper(hObj);
}

CGameObject* CGameInstance::Find_GameObject(const std::string& strName)
{
    return SYS_GAMEOBJECT.Find_GameObject(strName);
}

CGameObject* CGameInstance::Instantiate(const string& strProto, Layer::LAYER_ID iLayer, const string& strName, CGameObject* pParent)
{
    return SYS_GAMEOBJECT.Instantiate(strProto, iLayer, strName, pParent);
}

CGameObject* CGameInstance::Instantiate(const ASSET_GUID& tGUID, Layer::LAYER_ID iLayer, const string& strName, CGameObject* pParent)
{
    return SYS_GAMEOBJECT.Instantiate(tGUID, iLayer, strName, pParent);
}

_bool CGameInstance::Raycast(const POINT& pt, RAY& tRAY, RAYCAST_HIT& tHitInfo)
{
    RAYCAST_HITS allHits{};
    CRaycast::Intersect_Ray(allHits, tRAY, pt, m_pPhysics);

    if (allHits.iNumHits > 0)
    {
        tHitInfo = allHits.primaryHit;
        return true;
    }
    return false;
}

_bool CGameInstance::RaycastAll(const POINT& pt, RAY& tRAY, RAYCAST_HITS& tAllHitInfo)
{
    CRaycast::Intersect_Ray(tAllHitInfo, tRAY, pt, m_pPhysics);

    if (tAllHitInfo.iNumHits > 0)
        return true;

    return false;
}

_float CGameInstance::Get_DT() const noexcept
{
    const _float fDT = SYS_CORE.Get_FrameDT();
    return fDT;
}

void CGameInstance::Set_TimeScale()
{
}

void CGameInstance::Pause()
{
}

void CGameInstance::Play()
{
}

_float3 CGameInstance::Cam_Position() const
{
    return SYS_RENDER.Contexts()->Get_CamPosition();
}

_float3 CGameInstance::Cam_Look() const 
{
    _float4x4 matInvView = SYS_RENDER.Contexts()->Get_ViewInv();
    return _float3(matInvView._31, matInvView._32, matInvView._33);
}

const _float4x4& CGameInstance::Get_View() const
{
    return SYS_RENDER.Contexts()->Get_View();
}

const _float4x4& CGameInstance::Get_Proj() const
{
    return SYS_RENDER.Contexts()->Get_Proj();
}

const UI_GLOBAL& CGameInstance::Get_UI_Global() const
{
    return SYS_RENDER.Contexts()->Get_UI_Global();
}

const D3D11_VIEWPORT& CGameInstance::Get_Viewport() const
{
    D3D11_VIEWPORT vp{};
    UINT n = 1;
    m_pContext->RSGetViewports(&n, &vp);
    return vp;
}

_bool CGameInstance::Find_AttachBoneInfo(OBJECT_HANDLE hTargetObj, const string& strTargetBoneName, ANIMATOR_DATA*& pOutAnimator, _uint& iOutBoneIndex)
{
    CGameObject* pObj = SYS_GAMEOBJECT.Get_Wrapper(hTargetObj);
    IF_NULL_RETURN_MSG_BREAK(pObj, false, "pObj is nullptr");

    auto mr = pObj->Get_Component<CMeshRenderer>();
    auto anim = pObj->Get_Component<CAnimator>();

    IF_TRUE_RETURN_MSG_BREAK(mr.Is_Valid() == false, false, "mr is invalid");
    IF_TRUE_RETURN_MSG_BREAK(anim.Is_Valid() == false, false, "anim is invalid");

    if (!SYS_RESOURCE.Is_ModelHandle(mr->hMesh))
        return false;

    MODEL_ENTRY* pModel = SYS_RESOURCE.Get_Model(mr->hMesh);
    IF_NULL_RETURN_MSG_BREAK(pModel, false, "pModel is nullptr");

    auto it = pModel->tSkeleton.BoneNameToIndex.find(strTargetBoneName);
    if (it == pModel->tSkeleton.BoneNameToIndex.end())
        return false;

    /* 애니메이터 컴포넌트의 주소 + 해당 뼈의 인덱스 반환 */
    pOutAnimator = anim._Data();
    iOutBoneIndex = it->second;
    return true;
}

void CGameInstance::Calculate_AttachBoneMatrixPtr(const CTransform& hTargetTrans, CTransform& hAttachTrans, const _float4x4* matCombinedPtr)
{
    IF_NULL_RETURN_MSG_BREAK(matCombinedPtr, , "matCombinedPtr is nullptr.");

    Math::Store(
        hAttachTrans->matWorld,
        Math::Load(hAttachTrans->matWorld) *
        Math::Load(*matCombinedPtr) *
        Math::Load(hTargetTrans->matWorld)
    );
}

unique_ptr<CLine> CGameInstance::Load_LineMesh(_uint iNumPoint, _float fThickness, LINE_TYPE eType)
{
    /* 라인 메쉬부터 생성 */
    MESH_ENTRY entry;
    CMeshBuilder::Create_RibbonLine_VtxCol(m_pDevice, entry, iNumPoint);
    uint32_t handle = SYS_RESOURCE.Register_MeshEntry(std::move(entry));

    /* 라인 클래스 생성하여 반환 */
    auto pLine = CLine::Create(m_pDevice, m_pContext, iNumPoint, fThickness, eType);

    return pLine;
}

uint32_t CGameInstance::Get_ResourceHandle(ASSET_TYPE eType, const ASSET_GUID& tGUID)
{
    switch (eType)
    {
    case ASSET_TYPE::TEXTURE:
        return SYS_RESOURCE.Load_Texture(tGUID);

    case ASSET_TYPE::MESH:
        return SYS_RESOURCE.Load_Mesh(tGUID);

    case ASSET_TYPE::SHADER:
        return SYS_RESOURCE.Load_Shader(tGUID);

    case ASSET_TYPE::MATERIAL:
        return SYS_RESOURCE.Load_Material(tGUID);

    case ASSET_TYPE::FONT:
        return SYS_RESOURCE.Load_Font(tGUID);

    default:
        return INVALID_HANDLE_UINT;
    }
}

uint32_t CGameInstance::Alloc_PerObjectParamBlock()
{
    return SYS_RESOURCE.Alloc_PerObjectParamBlock();
}

PER_OBJECT_PARAM_BLOCK* CGameInstance::Get_PerObjectParamBlock(uint32_t handle)
{
    return SYS_RESOURCE.Get_PerObjectParamBlock(handle);;
}

std::unique_ptr<CNavMesh> CGameInstance::Create_NavMesh(const wchar_t* pFilePath, _bool bDebugRender)
{
    if (pFilePath == nullptr)
        return nullptr;

    std::unique_ptr<CNavMesh> upNavMesh = std::make_unique<CNavMesh>();
    if (!upNavMesh)
        return nullptr;

    if (false == upNavMesh->Load(pFilePath))
        return nullptr;

    SYS_EDITOR.Add_NavDebugRenderer(upNavMesh.get());

    return upNavMesh;
}

_bool CGameInstance::Read_GameConfig()
{
    std::filesystem::path configPath = "../../Game/Bin/Config/GameConfig.json";
    configPath.make_preferred();

    if (!std::filesystem::exists(configPath))
    {
        _DEBUG_WARN("GameConfig not found. Use default scene. path=%s", configPath.string().c_str());
        m_tGameConfig.startSceneName = "Untitled.scene";
        return S_OK;
    }

    std::ifstream ifs(configPath);
    if (!ifs.is_open())
    {
        _DEBUG_ERROR("GameConfig open failed. path=%s", configPath.string().c_str());
        m_tGameConfig.startSceneName = "Untitled.scene";
        return S_OK;
    }

    json root;
    try
    {
        ifs >> root;
    }
    catch (...)
    {
        _DEBUG_ERROR("GameConfig parse failed. path=%s", configPath.string().c_str());
        m_tGameConfig.startSceneName = "Untitled.scene";
        return S_OK;
    }

    const std::string startScene = root.value("startSceneName", "Untitled.scene");
    m_tGameConfig.startSceneName = startScene;

    return S_OK;
}

_bool CGameInstance::Resolve_SceneGUID(const std::string& strScene, ASSET_GUID& outGUID)
{
    outGUID = ASSET_GUID{};

    IF_TRUE_RETURN_MSG_BREAK(strScene.empty(), false, "Resolve_Scene_GUID failed: empty scene name");

    /* 시작 .scene 읽을 경로 */
    std::filesystem::path SCENE_PATH = ProjectConfig::PATH + ProjectConfig::ROOT;
    SCENE_PATH /= "Scenes";
    SCENE_PATH /= strScene;

    /* 확장자가 .scene 보정 */
    if (SCENE_PATH.extension().empty())
        SCENE_PATH.replace_extension(".scene");

    SCENE_PATH.make_preferred();
    SCENE_PATH = SYS_ASSET.Normalize_Path(SCENE_PATH);

    IF_TRUE_RETURN_MSG_BREAK(!std::filesystem::exists(SCENE_PATH), false,
        "Resolve_Scene_GUID failed: scene file not found");

    ASSET_GUID tGUID{};
    if (!SYS_ASSET.Try_Get_GUID(SCENE_PATH, tGUID) || !tGUID.Is_Valid())
    {
        const _bool ok = SYS_ASSET.Register_File_Asset(SCENE_PATH, ASSET_TYPE::SCENE, ASSET_GUID{});
        IF_TRUE_RETURN_MSG_BREAK(!ok, false, "Resolve_Scene_GUID failed: registry register failed");

        (void)SYS_ASSET.Try_Get_GUID(SCENE_PATH, tGUID);
        IF_TRUE_RETURN_MSG_BREAK(!tGUID.Is_Valid(), false, "Resolve_Scene_GUID failed: guid invalid after register");
    }

    outGUID = tGUID;
    return true;
}

void CGameInstance::Set_PostProcessDesc(const POST_PROCESS_DESC& tPostProcessDesc)
{
    SYS_RENDER.Set_PostProcessDesc(tPostProcessDesc);
}
