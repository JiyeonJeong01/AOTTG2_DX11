#include "Scene_Handler.h"

#include "Component_Spec.h"
#include "Core_System.h"
#include "Component_System.h"
#include "GameObject_System.h"
#include "Prototype_System.h"
#include "Scene.h"

NS_BEGIN(Engine)

CScene_Handler::CScene_Handler()
{
}

CScene_Handler::~CScene_Handler()
{
}

HRESULT CScene_Handler::Change_Scene(_uint iNewSceneIndex, std::unique_ptr<CScene> pNewScene)
{
    if (nullptr != m_pCurrentScene)
        SYS_CORE.Clear_Resources(m_iCurrentSceneIndex);

    m_pCurrentScene = std::move(pNewScene);
    m_iCurrentSceneIndex = iNewSceneIndex;

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

json CScene_Handler::Serialize_SceneObjectSpec(const SCENE_OBJECT_SPEC& tSpec)
{
    json j;
    j["uuid"] = tSpec.uuid.To_String_Utf8();
    j["protoGuid"] = tSpec.protoGuid.To_String_Utf8();
    j["name"] = tSpec.name;
    j["layer"] = (uint32_t)tSpec.layer;
    j["parent"] = tSpec.parent.Is_Valid() ? tSpec.parent.To_String_Utf8() : "";

    // overrides
    json jOverrides = json::array();
    for (const auto& up : tSpec.overrides.components)
    {
        if (!up)
            continue;

        json jc;
        jc["type"] = (uint32_t)up->Get_Type();
        json payload;
        up->ToJson(payload);
        jc["data"] = payload;

        jOverrides.push_back(std::move(jc));
    }
    j["overrides"] = std::move(jOverrides);
    return j;
}

_bool CScene_Handler::Deserialize_SceneObjectSpec(const json& j, SCENE_OBJECT_SPEC& out, SpecFactoryFn createSpec)
{
    if (!INSTANCE_UUID::Try_Utf8_To_UUID(j.value("uuid", ""), out.uuid))
        return false;
    
    if (!ASSET_GUID::Try_Utf8_To_GUID(j.value("protoGuid", ""), out.protoGuid))
        return false;

    out.name = j.value("name", "");
    out.layer = (Layer::LAYER_ID)j.value("layer", (uint32_t)Layer::DEFAULT_LAYER);

    /* parent uuid can be null */
    const std::string parentStr = j.value("parent", "");
    if (!parentStr.empty())
        INSTANCE_UUID::Try_Utf8_To_UUID(parentStr, out.parent);
    else
        out.parent = {}; // invalid

    out.overrides.components.clear();
    if (j.contains("overrides") && j["overrides"].is_array())
    {
        for (const auto& jc : j["overrides"])
        {
            const auto typeU32 = jc.value("type", (uint32_t)COMPONENT_TYPE::END);
            const auto eType = (COMPONENT_TYPE)typeU32;
            if (eType == COMPONENT_TYPE::END) continue;

            if (!jc.contains("data"))
            {
                _DEBUG_WARN("Override missing data; type=%u", (uint32_t)eType);
                continue;
            }

            auto spec = createSpec(eType);
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

            out.overrides.components.push_back(std::move(spec));
        }
    }
    return true;
}

HRESULT CScene_Handler::Apply_Overrides(CGameObject* pObject, const COMPONENT_SPEC_BUNDLE& tBundle)
{
    for (const auto& upSpec : tBundle.components)
    {
        /* TODO : 성공 여부 반환해야 함 그러려면 시스템 쪽에서 팩토리 자체 수정 필요  */
        if (!upSpec)
        {
            _DEBUG_WARN("COMPONENT_SPEC is nullptr; so skip this.");
            continue;
        }
        SYS_COMPONENT.Create_Component_From_Spec(pObject, upSpec.get());
    }

    return S_OK;
}

std::unique_ptr<COMPONENT_SPEC_BASE> CScene_Handler::Create_Spec_By_Type(COMPONENT_TYPE eType)
{
    switch (eType)
    {
    case COMPONENT_TYPE::TRANSFORM: return std::make_unique<TRANSFORM_SPEC>();
    default:
        return nullptr;
    }
}

HRESULT CScene_Handler::LoadScene_Runtime(const std::vector<SCENE_OBJECT_SPEC>& tSpecs)
{
    std::unordered_set<ASSET_GUID, ASSET_GUID_HASHER> protoGUIDs;
    protoGUIDs.reserve(tSpecs.size()); /* max size */

    for (const auto& spec : tSpecs)
        protoGUIDs.insert(spec.protoGuid);

    for (const auto& tGUID : protoGUIDs)
        if (FAILED(CPrototype_System::GetInstance().Load_Prototype_From_File(tGUID)))
            return E_FAIL;

    std::unordered_map<INSTANCE_UUID, CGameObject*, INSTANCE_UUID_HASHER> objectMap;
    objectMap.reserve(tSpecs.size());
    for (const auto& spec : tSpecs)
    {
        CGameObject* pSceneObj = CPrototype_System::GetInstance().Clone(spec.protoGuid, spec.layer, spec.name, spec.uuid);

        IF_NULL_RETURN_MSG_BREAK(pSceneObj, E_FAIL, "Prototype clone failed : GameObject is nullptr.");

        if (FAILED(Apply_Overrides(pSceneObj, spec.overrides)))
            continue;

        auto [it, bInserted] = objectMap.emplace(spec.uuid, pSceneObj);
        IF_TRUE_RETURN_MSG_BREAK(!bInserted, E_FAIL, "Duplicated UUID");
    }

    for (const auto& spec : tSpecs)
    {
        if (!spec.parent.Is_Valid())
            continue;

        auto itChild = objectMap.find(spec.uuid);
        auto itParent = objectMap.find(spec.parent);

        if (itChild == objectMap.end())
            _DEBUG_ERROR_BREAK("Child UUID not found.");
        if (itParent == objectMap.end())
            _DEBUG_ERROR_BREAK("Parent UUID not found.");

        if (itChild != objectMap.end() && itParent != objectMap.end())
            itChild->second->Set_Parent(itParent->second);
        else
            return E_FAIL;
    }

    return S_OK;
}

_bool CScene_Handler::Save_CurrentScene(const std::filesystem::path& path)
{
    std::vector<SCENE_OBJECT_SPEC> specs;


    return true;
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

_bool CScene_Handler::Load_SceneFile(std::vector<SCENE_OBJECT_SPEC>& outObjectSpecs, const std::filesystem::path& path, SpecFactoryFn createSpec)
{
    std::ifstream ifs(path);
    if (!ifs.is_open()) return false;

    json root;
    ifs >> root;

    outObjectSpecs.clear();
    if (!root.contains("objects") || !root["objects"].is_array())
        return false;

    for (const auto& jo : root["objects"])
    {
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
