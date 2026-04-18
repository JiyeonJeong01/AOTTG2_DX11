#include "Prototype_Handler.h"
#include "Engine_Log.h"
#include "Asset_Registry.h"
#include "Component_Spec.h"
#include "Prototype.h"
#include "GameObject.h"

#include "Spec_Struct.h"
#include "Component_Struct.h"

NS_BEGIN(Engine)

CPrototype_Handler::CPrototype_Handler()
{
    
}
CPrototype_Handler::~CPrototype_Handler()
{
    
}

HRESULT CPrototype_Handler::Initialize()
{
    m_pathProto = ProjectConfig::PATH + ProjectConfig::ROOT + "/Prototypes";

    return S_OK;
}


void CPrototype_Handler::Clear()
{
    m_Prototypes.clear();
}

/* Asset_Regigstry::Distribute_Assets_To_Handlers 에서 프로토타입 캐싱을 위해 호출된다. */
HRESULT CPrototype_Handler::Register_Prototype(const ASSET_GUID& tGUID, PROTOTYPE_SPEC&& spec)
{
    if (!tGUID.Is_Valid()) return E_FAIL;

    if (m_Prototypes.find(tGUID) != m_Prototypes.end())
    {
        _DEBUG_ENGINE_ASSERT_MSG(false, "Prototype key already exists!");
        return E_FAIL;
    }

    /* Prototype 객체 생성 및 조립 */
    auto pProto = CPrototype::Create();
    string strOriginalName = spec.strName;

    IF_FAIL_RETURN_MSG_BREAK(pProto->Assemble(tGUID, std::move(spec)), E_FAIL, "Assemble prototype failed");

    /* 이름 - 프로토타입 매핑 등록하기 */
    auto [nameIt, bSuccess] = m_NameToProto.emplace(strOriginalName, pProto.get());

    if (!bSuccess)
    {
        _DEBUG_ERROR_BREAK("Duplicate Prototype name. Renaming...");
        string newName = "Prototype_" + to_string(++m_iDuplicatedProtos);
        pProto->Set_Label(newName);
        m_NameToProto.emplace(newName, pProto.get());
    }

    /* 메인 매핑에 등록하며 최종 소유권 이동 */
    m_Prototypes.emplace(tGUID, std::move(pProto));

    return S_OK;
}

const CPrototype* CPrototype_Handler::Find(const ASSET_GUID& tGUID) const
{
    auto it = m_Prototypes.find(tGUID);
    if (it == m_Prototypes.end())
        return nullptr;
    return it->second.get();
}

const CPrototype* CPrototype_Handler::Find(const std::string& strName)
{
    auto it = m_NameToProto.find(strName);
    if (it == m_NameToProto.end())
        return nullptr;
    return it->second;
}

CGameObject* CPrototype_Handler::Clone(const ASSET_GUID& tGUID, Layer::LAYER_ID iLayer, const string& strName, const INSTANCE_UUID& tUUID) const
{
    const CPrototype* pProto = Find(tGUID);
    if (!pProto)
        return nullptr;

    return pProto->Clone(iLayer, strName, tUUID);
}

/* Editor에서 프로토타입 생성 시 호출된다 */
/* 원본 오브젝트를 반영하여 PROTOTYPE_SPEC을 작성한 뒤, 새로운 GUID를 만들어 시스템에 등록한다. */
_bool CPrototype_Handler::Create_Prototype_Spec(CGameObject* pObj)
{
    IF_TRUE_RETURN_MSG_BREAK(!pObj || !pObj->Is_Valid(), false, "Invalid pObj");

    PROTOTYPE_SPEC tSpec;
    Build_PrototypeSpec_From_Object(pObj, tSpec);

    std::filesystem::path savePath = m_pathProto / (tSpec.strName + ".proto");

    if (!Save_PrototypeFile(tSpec, savePath))
        return false;

    ASSET_GUID newGUID = ASSET_GUID::New_GUID();

    _bool bSuccess = SYS_ASSET.Register_File_Asset(savePath, ASSET_TYPE::PROTOTYPE, newGUID);
    IF_TRUE_RETURN_MSG_BREAK(!bSuccess, false, "Register_File_Asset failed");

    IF_TRUE_RETURN_MSG_BREAK(Load_Prototype_From_GUID(newGUID), false, "Load_Prototype_From_GUID failed"); /* 프로토타입 만든 직후 씬에 끌어오기 가능 */

    return true;
}

/* Editor에서 프로토타입 생성 시 호출된다 : CPrototype_Handler::Create_Prototype_Spec() -> */
void CPrototype_Handler::Build_PrototypeSpec_From_Object(CGameObject* pObj, PROTOTYPE_SPEC& outSpec)
{
    outSpec.strName = pObj->Get_Label();
    outSpec.isUI = pObj->Get_Handle().Is_UI();
    outSpec.iObjMask = pObj->Get_Mask();
    outSpec.tComponentBundle.Clear_All();
    outSpec.vecChildren.clear();

    Component::COMPONENT_MASK mask = pObj->Get_ComponentMask();
    for (_uint i = 0; i < COMPONENT_MAX; ++i)
    {
        if ((mask & Component::To_Bit(INT_TO_COM(i))) == 0)
            continue;

        vector<COMPONENT_HANDLE> hComponents;
        SYS_COMPONENT.Get_Component_Handle_By_Type(INT_TO_COM(i), pObj->Get_Handle(), hComponents);

        auto& vOut = outSpec.tComponentBundle.components[i];
        vOut.clear();
        vOut.reserve(hComponents.size());

        for (auto hCom : hComponents)
        {
            vOut.emplace_back(SYS_COMPONENT.Build_Spec_By_Type(INT_TO_COM(i), hCom));
        }

        __noop;
    }

    /* 자식 재귀로 채우기 */
    const auto& pChildren = pObj->Get_Children();
    outSpec.vecChildren.reserve(pChildren.size());

    for (auto* pChild : pChildren)
    {
        PROTOTYPE_SPEC childSpec;
        Build_PrototypeSpec_From_Object(pChild, childSpec);
        outSpec.vecChildren.push_back(std::move(childSpec));
    }
}

/* =========================================================
 * Save & Serialize
 * ========================================================= */
_bool CPrototype_Handler::Save_PrototypeFile(const PROTOTYPE_SPEC& tSpec, const std::filesystem::path& path)
{
    json root;
    root["version"] = 1;
    root["prototype"] = Serialize_PrototypeSpec(tSpec);

    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);

    std::ofstream ofs(path);
    if (!ofs.is_open())
        return false;

    /* 들여쓰기 2칸 */
    ofs << root.dump(2);
    return true;
}

json CPrototype_Handler::Serialize_PrototypeSpec(const PROTOTYPE_SPEC& tSpec)
{
    json j;

    j["name"] = tSpec.strName;
    j["isUI"] = tSpec.isUI;

    /* 컴포넌트 직렬화 (2차원: [COMPONENT_MAX][N]) */
    json jComponents = json::array();

    for (uint32_t i = 0; i < SCAST(uint32_t, COMPONENT_MAX); ++i)
    {
        json jSlot = json::array();

        const auto& vSpecs = tSpec.tComponentBundle.components[i];
        for (const auto& pSpec : vSpecs)
        {
            if (!pSpec) continue;

            json jEntry;
            json payload;
            pSpec->ToJson(payload);
            jEntry["data"] = std::move(payload);
            jSlot.push_back(std::move(jEntry));
        }

        jComponents.push_back(std::move(jSlot)); // 이게 핵심
    }

    j["iObjMask"] = tSpec.iObjMask;

    j["components"] = std::move(jComponents);

    if (!tSpec.vecChildren.empty())
    {
        json jChildren = json::array();
        for (const auto& child : tSpec.vecChildren)
        {
            jChildren.push_back(Serialize_PrototypeSpec(child));
        }
        j["children"] = std::move(jChildren);
    }

    return j;
}


/* =========================================================
 * Load & Deserialize
 * ========================================================= */

HRESULT CPrototype_Handler::Load_Prototype_From_GUID(const ASSET_GUID& tGUID)
{
    const ASSET_RECORD* pRec = SYS_ASSET.Find(tGUID);
    IF_NULL_RETURN_MSG_BREAK(pRec, E_FAIL, "Prototype guid not found in asset registry.");

    /* 스펙 로드 */
    PROTOTYPE_SPEC spec;
    if (!Load_PrototypeFile(spec, pRec->path))
    {
        _DEBUG_ERROR_BREAK("Load_PrototypeFile failed.");
        return E_FAIL;
    }

    /* 로드한 스펙을 메모리에 등록한다. */
    IF_FAIL_RETURN_MSG_BREAK(Register_Prototype(tGUID, std::move(spec)), E_FAIL, "Failed to register prototype from file.");

    return S_OK;
}

_bool CPrototype_Handler::Load_PrototypeFile(PROTOTYPE_SPEC& outSpec, const std::filesystem::path& path)
{
    std::ifstream ifs(path);
    if (!ifs.is_open()) return false;

    json root;
    try { ifs >> root; }
    catch (...) { return false; }

    if (!root.contains("prototype"))
        return false;

    if (!Deserialize_PrototypeSpec(root["prototype"], outSpec))
        return false;

    return true;
}

_bool CPrototype_Handler::Deserialize_PrototypeSpec(const json& j, PROTOTYPE_SPEC& outSpec)
{
    outSpec.strName = j.value("name", "Prototype");
    outSpec.isUI = j.value("isUI", false);
    outSpec.iObjMask = j.value("iObjMask", 0);

    /* 컴포넌트 번들 초기화 */
    outSpec.tComponentBundle.Clear_All();

    if (j.contains("components") && j["components"].is_array())
    {
        const auto& jComponents = j["components"];

        const uint32_t iCount = SCAST(uint32_t, jComponents.size());
        const uint32_t iMax = SCAST(uint32_t, COMPONENT_MAX);
        const uint32_t iLoop = (iCount < iMax) ? iCount : iMax;

        for (uint32_t i = 0; i < iLoop; ++i)
        {
            const COMPONENT_TYPE eComType = INT_TO_COM(i);

            const auto& jSlot = jComponents[i];
            if (!jSlot.is_array())
                continue;

            auto& vOut = outSpec.tComponentBundle.components[i];
            vOut.clear();
            vOut.reserve(jSlot.size());

            for (const auto& jEntry : jSlot)
            {
                if (!jEntry.contains("data"))
                    continue;

                auto spec = Create_Spec_By_Type(eComType);
                if (!spec)
                {
                    _DEBUG_ERROR_BREAK("Unknown prototype spec type=%u", i);
                    continue;
                }

                if (!spec->FromJson(jEntry["data"]))
                {
                    _DEBUG_ERROR_BREAK("Prototype Spec FromJson failed; type=%u", i);
                    continue;
                }

                vOut.emplace_back(std::move(spec));
            }
        }
    }

    outSpec.vecChildren.clear();
    if (j.contains("children") && j["children"].is_array())
    {
        outSpec.vecChildren.reserve(j["children"].size());
        for (const auto& jc : j["children"])
        {
            PROTOTYPE_SPEC child{};
            if (Deserialize_PrototypeSpec(jc, child))
            {
                outSpec.vecChildren.push_back(std::move(child));
            }
        }
    }

    return true;
}

/* SpecFactoryFn */
std::unique_ptr<COMPONENT_SPEC_BASE> CPrototype_Handler::Create_Spec_By_Type(COMPONENT_TYPE eType)
{
    /* ---------------------------------------------------------------------
    *  TODO : 컴포넌트 추가 시 반드시 추가
    * --------------------------------------------------------------------- */
    switch (eType)
    {
    case COMPONENT_TYPE::TRANSFORM: return std::make_unique<TRANSFORM_SPEC>();
    case COMPONENT_TYPE::COLLIDER: return std::make_unique<COLLIDER_SPEC>();
    case COMPONENT_TYPE::RIGIDBODY: return std::make_unique<RIGIDBODY_SPEC>();
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
    case COMPONENT_TYPE::SPRITE_EFFECT: return std::make_unique<SPRITE_EFFECT_SPEC>();

    default:
        return nullptr;
    }
}


std::unique_ptr<CPrototype_Handler> CPrototype_Handler::Create()
{
    auto pInstance = std::make_unique<CPrototype_Handler>();

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "Create instance failed");
    return pInstance;
}

NS_END
