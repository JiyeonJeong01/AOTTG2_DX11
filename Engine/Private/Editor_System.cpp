#include "Editor_System.h"
#include "Asset_Registry.h"
#include "Event_System.h"
#include "SceneChange_Event.h"
#include "Scene.h"
#include "Core_System.h"

IMPLEMENT_SINGLETON(CEditor_System)

CEditor_System::CEditor_System()
{
    
}
CEditor_System::~CEditor_System()
{
    
}


HRESULT CEditor_System::Initialize(const std::filesystem::path& assetRoot)
{
    m_pathAsset = assetRoot;

    SYS_EVENT.Subscribe(EVENT_TYPE::On_Scene_Changed, &CEditor_System::On_SceneChanged, this);

    return S_OK;
}

ASSET_GUID CEditor_System::Ensure_DefaultScene()
{
    IF_TRUE_RETURN_MSG_BREAK(m_pathAsset.empty(), ASSET_GUID{}, "Ensure_DefaultScene failed: assetRoot empty");

    const std::filesystem::path sceneDir = m_pathAsset / "Scenes";
    const std::filesystem::path scenePath = sceneDir / "Untitled.scene";

    std::error_code ec;
    std::filesystem::create_directories(sceneDir, ec);

    if (!std::filesystem::exists(scenePath))
    {
        json root;
        root["version"] = 1;
        root["objects"] = json::array();

        std::ofstream ofs(scenePath);
        IF_TRUE_RETURN_MSG_BREAK(!ofs.is_open(), ASSET_GUID{}, "Ensure_DefaultScene failed: cannot create Default.scene");
        ofs << root.dump(2);
    }

    /* 레지스트리에 등록 (.meta와 path<->guid 맵 보장) */
    SYS_ASSET.Register_File_Asset(scenePath, ASSET_TYPE::SCENE, ASSET_GUID{});

    ASSET_GUID guid{};
    if (!SYS_ASSET.Try_Get_GUID(scenePath, guid) || !guid.Is_Valid())
    {
        guid = Ensure_Asset_Has_Meta(SYS_ASSET.Normalize_Path(scenePath), SYS_ASSET.AssetType_ToStr(ASSET_TYPE::SCENE));
        (void)SYS_ASSET.Register_File_Asset(scenePath, ASSET_TYPE::SCENE, guid);
    }

    return guid;
}

ASSET_GUID CEditor_System::Create_NewScene_Asset(std::filesystem::path* outPath)
{
    IF_TRUE_RETURN_MSG_BREAK(m_pathAsset.empty(), ASSET_GUID{}, "Create_NewScene_Asset failed: assetRoot empty");

    const std::filesystem::path sceneDir = m_pathAsset / "Scenes";
    std::error_code ec;
    std::filesystem::create_directories(sceneDir, ec);
    std::filesystem::path path;

    for (int i = 1; i < 10000; ++i)
    {
        path = sceneDir / ("Untitled_" + std::to_string(i) + ".scene");
        if (!std::filesystem::exists(path))
            break;
    }

    json root;
    root["version"] = 1;
    root["objects"] = json::array();

    std::ofstream ofs(path);
    IF_TRUE_RETURN_MSG_BREAK(!ofs.is_open(), ASSET_GUID{}, "Create_NewScene_Asset failed: cannot create scene file");
    ofs << root.dump(2);

    ASSET_GUID guid = ASSET_GUID::New_GUID();
    IF_TRUE_RETURN_MSG_BREAK(!SYS_ASSET.Register_File_Asset(path, ASSET_TYPE::SCENE, guid), ASSET_GUID{}, "Create_NewScene_Asset failed: registry register failed");

    if (outPath) *outPath = path;
    return guid;
}

void CEditor_System::On_SceneChanged(EVENT_DATA& event)
{
    SCENECHANGE_EVENT_DATA& onSceneChanged = SCAST(SCENECHANGE_EVENT_DATA&, event);
    m_pCurScene = onSceneChanged.m_pNewScene;
}

void CEditor_System::Play()
{
    IF_NULL_RETURN_MSG_BREAK(m_pCurScene, , "m_pCurScene is nullptr");
    m_pCurScene->Set_State(SCENE_STATE::PLAY);
}

void CEditor_System::Pause()
{
    IF_NULL_RETURN_MSG_BREAK(m_pCurScene, , "m_pCurScene is nullptr");
    m_pCurScene->Set_State(SCENE_STATE::PAUSE);
}

void CEditor_System::Step(_float fDT)
{
    IF_NULL_RETURN_MSG_BREAK(m_pCurScene, , "m_pCurScene is nullptr");
    if (m_pCurScene->Get_State() == SCENE_STATE::PAUSE)
        SYS_CORE.Request_Step(fDT, m_pCurScene);
}
