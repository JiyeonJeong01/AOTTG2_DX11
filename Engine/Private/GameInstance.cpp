#include "GameInstance.h"

#include "Core_System.h"
#include "GameObject_System.h"
#include "Component_System.h"
#include "Input_System.h"
#include "Logger.h"

#include "Asset_Registry.h"
#include "Engine_Log.h"
#include "GameObject.h"

#include "Raycast.h"
#include "Physics_Processor.h"

IMPLEMENT_SINGLETON(CGameInstance)

CGameInstance::~CGameInstance()
{
}

CGameInstance::CGameInstance()
{
}

HRESULT CGameInstance::Initialize()
{
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
    IF_FAIL_RETURN_MSG_BREAK(SYS_CORE.Change_Scene(outGUID, APP_MODE::GAME_PLAY), E_FAIL, "SetUp_Game failed: Change_Scene failed");

    return S_OK;
}

void CGameInstance::Change_Scene(const std::string& strScene)
{
    /* 반드시 .scene 의 이름과 동일해야 한다. */
}

CGameObject* CGameInstance::Get_GameObject(OBJECT_HANDLE hObj)
{
    return nullptr;
}

CGameObject* CGameInstance::Get_GameObject(COMPONENT_HANDLE hComponent)
{
    return nullptr;
}

CGameObject* CGameInstance::Find_GameObject(const std::string& strName)
{
    return nullptr;
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

void CGameInstance::Test_Raycast()
{
    RAYCAST_HITS allHitInfo{};
    RAY tRAY;

    if (SYS_INPUT.Get_KeyDown('I'))
    {
        tRAY.fMaxDist = 1000.f;
        tRAY.fMinDist = 0.f;

        LOG_INFO("[ RAYCAST TEST ] : max dist - %.1f, min dist - %.1f",
            tRAY.fMaxDist, tRAY.fMinDist);

        CRaycast::Intersect_Ray(allHitInfo, tRAY, SYS_INPUT.Get_GameMousePos(), m_pPhysics);
    }

    if (SYS_INPUT.Get_KeyDown('O'))
    {
        tRAY.fMaxDist = 50.f;
        tRAY.fMinDist = 0.f;

        LOG_INFO("[ RAYCAST TEST ] : max dist - %.1f, min dist - %.1f",
            tRAY.fMaxDist, tRAY.fMinDist);

        CRaycast::Intersect_Ray(allHitInfo, tRAY, SYS_INPUT.Get_GameMousePos(), m_pPhysics);
    }

    if (SYS_INPUT.Get_KeyDown('P'))
    {
        tRAY.fMaxDist = 1000.f;
        tRAY.fMinDist = 50.f;

        LOG_INFO("[ RAYCAST TEST ] : max dist - %.1f, min dist - %.1f",
            tRAY.fMaxDist, tRAY.fMinDist);

        CRaycast::Intersect_Ray(allHitInfo, tRAY, SYS_INPUT.Get_GameMousePos(), m_pPhysics);
    }

    if (allHitInfo.iNumHits > 0)
    {
        _uint i = 0;
        for (auto hit : allHitInfo.allHits)
        {
            CGameObject* pObject = SYS_GAMEOBJECT.Get_Wrapper(hit.hObject);
            LOG_INFO("%d : Object : { %.*s } | Hit Pos : { %.1f, %.1f, %.1f }",
                i,
                pObject->Get_Label().data(),
                hit.vHitPos.x, hit.vHitPos.y, hit.vHitPos.z);
        }
    }
}

_float CGameInstance::Get_DT() const noexcept
{
    return 0.f;
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
