#include "Scene_Handler.h"

#include "Component_Spec.h"
#include "Core_System.h"
#include "Component_System.h"
#include "GameObject_System.h"
#include "Event_System.h"
#include "Scene.h"
#include "Asset_Registry.h"
#include "MeshRenderer.h"
#include "SceneChange_Event.h"
#include "MeshRenderer_Processor.h"
#include "Script_Processor.h"

NS_BEGIN(Engine)

CScene_Handler::CScene_Handler()
{
}

CScene_Handler::~CScene_Handler()
{
}

HRESULT CScene_Handler::Register_Scenes(const ASSET_GUID& tGUID, const std::filesystem::path& scenePath)
{
    const std::string name = scenePath.stem().string();
    if (name.empty())
        return E_FAIL;

    auto it = m_NameToGUID.find(name);
    if (it != m_NameToGUID.end()) /* 이름 중복 시, 덮어쓰기 */
    {
        _DEBUG_INFO_BREAK("Scene name duplicated: %s (overwrite)", name.c_str());
        it->second = tGUID;
        return S_OK;
    }
    m_NameToGUID.emplace(name, tGUID);
    return S_OK;
}

_bool CScene_Handler::Find_GUID_By_Name(const std::string& name, ASSET_GUID& outGUID) const
{
    /* ~.scene으로 들어올 경우 대비하여 정규화*/
    std::string key = name;
    {
        std::filesystem::path p = key;
        if (p.has_extension())
            key = p.stem().string();
    }

    auto it = m_NameToGUID.find(name);
    if (it == m_NameToGUID.end())
        return false;

    outGUID = it->second;
    return true;
}

HRESULT CScene_Handler::Change_Scene(const ASSET_GUID& tGUID)
{
    IF_TRUE_RETURN_MSG_BREAK(!tGUID.Is_Valid(), E_FAIL, "Invalid scene GUID");

    /* Resolve GUID */
    /* 에디터 모드에서 새 씬 생성 시 새 GUID를 생성하므로 아래 에러를 통과한다. */
    const ASSET_RECORD* pRec = SYS_ASSET.Find(tGUID);
    IF_NULL_RETURN_MSG_BREAK(pRec, E_FAIL, "Scene guid not found in asset registry.");
    IF_TRUE_RETURN_MSG_BREAK(pRec->eType != ASSET_TYPE::SCENE, E_FAIL, "Not a scene asset.");

    const std::filesystem::path& path = pRec->path;
    IF_TRUE_RETURN_MSG_BREAK(path.empty(), E_FAIL, "Scene record path is empty.");

    // 기존 씬 정리 (mode에 따라 다르게)
    // SYS_GAMEOBJECT.Destory_All_GameObjects();

    auto pNewScene = CScene::Create();
    auto pOldScene = std::move(m_pCurrentScene);

    m_pCurrentScene = std::move(pNewScene);

    if (FAILED(Load_NextScene(path, tGUID))) // Load_SceneFile(...) -> LoadScene_Runtime(...)
    {
        m_pCurrentScene = std::move(pOldScene);
        return E_FAIL;
    }

    // current guid 갱신
    m_pCurrentScene->Set_GUID(tGUID);
    m_pCurrentScene->Set_Label(path.stem().string());
    m_pCurrentScene->Set_State(SCENE_STATE::PLAY);

    /* --- TODO (Optional) Handle Editor/Game scene change logic */

    /* Publish event */
    SCENECHANGE_EVENT_DATA onSceneChange(EVENT_TYPE::On_Scene_Changed, m_pCurrentScene.get(), pOldScene.get());
    SYS_EVENT.Trigger(onSceneChange);


    return S_OK;
}

HRESULT CScene_Handler::Open_EditScene(const ASSET_GUID& tGUID)
{
    IF_FAIL_RETURN_MSG_BREAK(Change_Scene(tGUID), E_FAIL, "change scene failed");

    m_pCurrentScene->Set_State(SCENE_STATE::EDIT);

    return S_OK;
}

void CScene_Handler::Update(_float fTimeDelta)
{
    if (nullptr != m_pCurrentScene)
        m_pCurrentScene->Update(fTimeDelta);
}

HRESULT CScene_Handler::Render()
{
    if (nullptr != m_pCurrentScene)
        m_pCurrentScene->Render();

    return S_OK;
}

void CScene_Handler::Set_CurrentScene(std::unique_ptr<CScene> pScene)
{
    m_pCurrentScene = std::move(pScene);
}

CScene* CScene_Handler::Get_CurrentScene()
{
    return m_pCurrentScene.get();
}

/* Save_CurrentScene -> Save_SceneFile -> Serialize_SceneObjectSpec() */
_bool CScene_Handler::Save_CurrentScene(const std::filesystem::path& path)
{
    CScene& scene = *m_pCurrentScene;
    IF_NULL_RETURN_MSG_BREAK(m_pCurrentScene, false, "m_pCurrentScene is nullptr");

    if (!scene.Get_GUID().Is_Valid())
        scene.Set_GUID(ASSET_GUID::New_GUID());

    std::vector<SCENE_OBJECT_SPEC> specs;

    IF_FAIL_RETURN_MSG_BREAK(SYS_GAMEOBJECT.Build_SceneSpecs(specs), false, "GameObject_System failed to serialize");

    /* Ensure save path */
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);

    if (!Save_SceneFile(specs, path))
        return false;

    IF_TRUE_RETURN_MSG_BREAK(!SYS_ASSET.Register_File_Asset(path, ASSET_TYPE::SCENE, scene.Get_GUID()), false, "Register scene to Asset_Registry failed");

    return true;
}

/* Called by CScene_Handler::Change_Scene() */
/* Load_NextScene -> Load_SceneFile -> Deserialize_SceneObjectSpec() */
_bool CScene_Handler::Load_NextScene(const std::filesystem::path& path, const ASSET_GUID& tGUID)
{
    IF_TRUE_RETURN_MSG_BREAK(path.empty() || !std::filesystem::exists(path), false, "Load_NextScene failed: path invalid.");

    /* .scene 파일의 직렬화된 SCENE_OBJECT_SPEC을 런타임 객체로 생성한다. */
    std::vector<SCENE_OBJECT_SPEC> specs;
    if (!Load_SceneFile(specs, path, [](COMPONENT_TYPE t) { return Create_Spec_By_Type(t); }))
    {
        _DEBUG_ERROR_BREAK("Load_SceneFile failed.");
        return false;
    }

    SYS_GAMEOBJECT.Destroy_All_SceneObjects();
    /* ------------------------------------------*/
    /* TODO 기타 씬 정리 구현 : 아직 리소스가 없다. */
    /* ------------------------------------------*/

    IF_FAIL_RETURN_MSG_BREAK(LoadScene_Runtime(specs), false, "LoadScene_Runtime failed");

    return true;
}

_bool CScene_Handler::Restart()
{
    if (nullptr == m_pCurrentScene)
        return false;

    HRESULT hr = Open_EditScene(m_pCurrentScene->Get_GUID());

    if (hr == E_FAIL) return false;
    else return true;
}

_bool CScene_Handler::Save_SceneFile(const std::vector<SCENE_OBJECT_SPEC>& objects, const std::filesystem::path& path)
{
    json root;
    root["version"] = 1;
    root["objects"] = json::array();

    for (const auto& o : objects)
        root["objects"].push_back(Serialize_SceneObjectSpec(o));

    std::ofstream ofs(path);
    if (!ofs.is_open())
        return false;
    ofs << root.dump(2);
    return true;
}

json CScene_Handler::Serialize_SceneObjectSpec(const SCENE_OBJECT_SPEC& tSpec)
{
    json j;
    j["uuid"] = tSpec.uuid.To_String_Utf8();
    j["protoGuid"] = tSpec.protoGuid.To_String_Utf8();
    j["isUI"] = tSpec.isUI;
    j["name"] = tSpec.name;
    j["layer"] = (uint32_t)tSpec.layer;
    j["parent"] = tSpec.parent.Is_Valid() ? tSpec.parent.To_String_Utf8() : "";

    /* overrides sparse flat list */ 
    json jOverrides = json::array();

    for (uint32_t i = 0; i < SCAST(uint32_t, COMPONENT_MAX); ++i)
    {
        const auto& vSpecs = tSpec.overrides.components[i];
        if (vSpecs.empty())
            continue;

        for (const auto& up : vSpecs)
        {
            if (!up)
                continue;

            json jc;
            jc["type"] = (uint32_t)up->Get_Type();

            json payload;
            up->ToJson(payload);
            jc["data"] = std::move(payload);

            jOverrides.push_back(std::move(jc));
        }
    }

    j["overrides"] = std::move(jOverrides);
    return j;
}

/* Json의 직렬화된 내용을 SCENE_OBJECT_SPEC 구조체로 빌드한다. */
_bool CScene_Handler::Deserialize_SceneObjectSpec(const json& j, SCENE_OBJECT_SPEC& out, SpecFactoryFn createSpec)
{
    if (!INSTANCE_UUID::Try_Utf8_To_UUID(j.value("uuid", ""), out.uuid))
        return false;

    if (!ASSET_GUID::Try_Utf8_To_GUID(j.value("protoGuid", ""), out.protoGuid))
        out.protoGuid = ASSET_GUID{};

    out.isUI = j.value("isUI", false);
    out.name = j.value("name", "");
    out.layer = (Layer::LAYER_ID)j.value("layer", (uint32_t)Layer::DEFAULT_LAYER);

    /* 부모 UUID는 비어있을 수 있다. */
    const std::string parentStr = j.value("parent", "");
    if (!parentStr.empty())
        INSTANCE_UUID::Try_Utf8_To_UUID(parentStr, out.parent);
    else
        out.parent = {}; // invalid

    /* overrides 초기화 */
    out.overrides.Clear_All();

    if (j.contains("overrides") && j["overrides"].is_array())
    {
        for (const auto& jc : j["overrides"])
        {
            const auto typeU32 = jc.value("type", (uint32_t)COMPONENT_TYPE::END);
            const auto eType = (COMPONENT_TYPE)typeU32;
            if (eType == COMPONENT_TYPE::END)
                continue;

            const uint32_t iSlot = SCAST(uint32_t, eType);
            if (iSlot >= SCAST(uint32_t, COMPONENT_MAX))
                continue;

            if (!jc.contains("data"))
            {
                _DEBUG_WARN("Override missing data; type=%u", (uint32_t)eType);
                continue;
            }

            auto spec = createSpec(eType); /* 컴포넌트에 맞는 COMPONENT_SPEC_BASE의 구현체 생성 */
            if (!spec)
            {
                _DEBUG_WARN("Unknown spec type=%u", (uint32_t)eType);
                continue;
            }

            if (!spec->FromJson(jc["data"]))
            {
                _DEBUG_WARN("Spec FromJson failed; type=%u", (uint32_t)eType);
                continue;
            }

            /* 같은 타입 슬롯에 push하여 그룹(Primary/Extras) 형태로 빌드한다. */ 
            out.overrides.components[iSlot].emplace_back(std::move(spec));
        }
    }

    return true;
}

/* 빌드된 스펙으로 오브젝트가 할당받은 컴포넌트에 override 한다. */
HRESULT CScene_Handler::Apply_Overrides(CGameObject* pObject, const COMPONENT_SPEC_BUNDLE& tBundle)
{
    IF_NULL_RETURN_MSG_BREAK(pObject, E_FAIL, "pObj is nullptr");

    for (uint32_t i = 0; i < SCAST(uint32_t, COMPONENT_MAX); ++i)
    {
        const auto& vSpecs = tBundle.components[i];
        if (vSpecs.empty())
            continue;

        for (const auto& upSpec : vSpecs)
        {
            if (!upSpec)
                continue;

            IF_FAIL_RETURN_MSG_BREAK(SYS_COMPONENT.Create_Component_From_Spec(pObject, upSpec.get()), E_FAIL,
                "Apply overrides failed");
        }
    }

    return S_OK;
}

std::unique_ptr<COMPONENT_SPEC_BASE> CScene_Handler::Create_Spec_By_Type(COMPONENT_TYPE eType)
{
    /* ---------------------------------------------------------------------
     *  TODO : 컴포넌트 추가 시 반드시 추가 
     * --------------------------------------------------------------------- */
    switch (eType)
    {
    case COMPONENT_TYPE::TRANSFORM: return std::make_unique<TRANSFORM_SPEC>();
    case COMPONENT_TYPE::RIGIDBODY: return std::make_unique<RIGIDBODY_SPEC>();
    case COMPONENT_TYPE::COLLIDER: return std::make_unique<COLLIDER_SPEC>();
    case COMPONENT_TYPE::SPRING_JOINT: return std::make_unique<SPRING_JOINT_SPEC>();
    case COMPONENT_TYPE::CAMERA: return std::make_unique<CAMERA_SPEC>();
    case COMPONENT_TYPE::ANIMATOR: return std::make_unique<ANIMATOR_SPEC>();
    case COMPONENT_TYPE::RECT_TRANSFORM: return std::make_unique<RECTTRANSFORM_SPEC>();
    case COMPONENT_TYPE::CANVAS_RENDERER: return std::make_unique<CANVAS_RENDERER_SPEC>();
    case COMPONENT_TYPE::MESH_RENDERER: return std::make_unique<MESH_RENDERER_SPEC>();
    case COMPONENT_TYPE::SCRIPT: return std::make_unique<SCRIPT_SPEC>();
    case COMPONENT_TYPE::UI_IMAGE: return std::make_unique<UI_IMAGE_SPEC>();
    case COMPONENT_TYPE::UI_BUTTON: return std::make_unique<UI_BUTTON_SPEC>();
    case COMPONENT_TYPE::UI_TEXT: return std::make_unique<UI_TEXT_SPEC>();
    default:
        return nullptr;
    }
}

HRESULT CScene_Handler::LoadScene_Runtime(const std::vector<SCENE_OBJECT_SPEC>& tSpecs)
{
    std::unordered_map<INSTANCE_UUID, CGameObject*, INSTANCE_UUID_HASHER> objectMap;
    std::vector<CGameObject*> stagingObjects;
    objectMap.reserve(tSpecs.size());
    stagingObjects.reserve(tSpecs.size());

    /* 오브젝트 생성만 먼저 */
    for (const auto& spec : tSpecs)
    {
        CGameObject* pObj = nullptr;
        if (!spec.isUI)
            pObj = SYS_GAMEOBJECT.Create_GameObject(spec.layer, spec.name, nullptr, spec.uuid);
        else
            pObj = SYS_GAMEOBJECT.Create_GameObjectUI(Layer::UI_LAYER, spec.name, nullptr, spec.uuid);

        IF_NULL_RETURN_MSG_BREAK(pObj, E_FAIL, "Create_Object failed.");

        pObj->Set_ProtoGUID(spec.protoGuid);

        auto [it, inserted] = objectMap.emplace(spec.uuid, pObj);
        stagingObjects.push_back(pObj);

        SYS_GAMEOBJECT.Register_UUID_Handle(spec.uuid, pObj->Get_Handle());

        // IF_TRUE_RETURN_MSG_BREAK(!inserted, E_FAIL, "Duplicated UUID");
    }

    /* 부모 연결 먼저 */
    for (const auto& spec : tSpecs)
    {
        if (!spec.parent.Is_Valid())
            continue;

        auto itChild = objectMap.find(spec.uuid);
        auto itParent = objectMap.find(spec.parent);

        IF_TRUE_RETURN_MSG_BREAK(itChild == objectMap.end(), E_FAIL, "Child UUID not found.");
        IF_TRUE_RETURN_MSG_BREAK(itParent == objectMap.end(), E_FAIL, "Parent UUID not found.");

        itChild->second->Set_Parent(itParent->second);
    }

    /* 부모가 연결된 상태에서 override 적용 */
    for (const auto& spec : tSpecs)
    {
        auto itObj = objectMap.find(spec.uuid);
        IF_TRUE_RETURN_MSG_BREAK(itObj == objectMap.end(), E_FAIL, "Object UUID not found.");

        IF_FAIL_RETURN_MSG_BREAK(Apply_Overrides(itObj->second, spec.overrides), E_FAIL, "Apply_Overrides failed.");
    }

    CMeshRenderer_Processor* pMeshrenderer_Processor = SYS_COMPONENT.Bind_Processor<CMeshRenderer_Processor>();
    IF_NULL_RETURN_MSG_BREAK(pMeshrenderer_Processor, E_FAIL, "can't bind with mesh renderer processor");

    /* 필요하면 한 번 더 참조 재해결 */
    for (auto pObj : stagingObjects)
    {
        if ((pObj->Get_ComponentMask() & Component::To_Bit(COMPONENT_TYPE::MESH_RENDERER)) == 0)
            continue;

        auto meshRenderer = pObj->Get_Component<CMeshRenderer>();
        if (!meshRenderer.Is_Valid())
            continue;

        if (meshRenderer->eMode == MESH_MODE::PARTS)
            pMeshrenderer_Processor->Resolve_SkinningReference(meshRenderer.Get_Handle());
        else if (meshRenderer->eMode == MESH_MODE::ATTACH)
            pMeshrenderer_Processor->Resolve_AttachReference(meshRenderer.Get_Handle());
    }

    /* 스크립트 컴포넌트가 저장한 UUID <-> 런타임 게임오브젝트 간 연결 */
    CScript_Processor* pScriptProcessor = SYS_COMPONENT.Bind_Processor<CScript_Processor>();
    IF_NULL_RETURN_MSG_BREAK(pScriptProcessor, E_FAIL, "can't bind with script processor");

    for (auto pObj : stagingObjects)
    {
        if ((pObj->Get_ComponentMask() & Component::To_Bit(COMPONENT_TYPE::SCRIPT)) == 0)
            continue;

        auto scripts = pObj->Get_Components<CScript>();
        for (auto sc : scripts)
        {
            IScript* pScript = pScriptProcessor->Get_Script_Instance(sc.Get_Handle());
            if (!pScript)
                continue;

            pScript->Resolve_Exposed_ObjectRefs();
        }
    }

    return S_OK;
}

_bool CScene_Handler::Load_SceneFile(std::vector<SCENE_OBJECT_SPEC>& outObjectSpecs, const std::filesystem::path& path, SpecFactoryFn createSpec)
{
    std::ifstream ifs(path);
    if (!ifs.is_open()) return false;

    json root;

    try { ifs >> root; }
    catch (...) { return false; }

    outObjectSpecs.clear();
    if (!root.contains("objects") || !root["objects"].is_array())
        return false;

    for (const auto& jo : root["objects"])
    {
        /* SCENE_OBJECT_SPEC의 멤버변수를 채우고, COMPONENT_SPEC_BASE*를 생성하여 값을 채우기 위한 그릇을 준비한다. */
        SCENE_OBJECT_SPEC s{};
        if (!Deserialize_SceneObjectSpec(jo, s, createSpec))
            return false;
        outObjectSpecs.push_back(std::move(s));
    }
    return true;
}

std::unique_ptr<CScene_Handler> CScene_Handler::Create()
{
    return std::make_unique<CScene_Handler>();
}

NS_END
