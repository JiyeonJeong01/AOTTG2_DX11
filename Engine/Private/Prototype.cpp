#include "Prototype.h"
#include "Engine_Log.h"
#include "GameObject_System.h"
#include "Component_System.h"
#include "GameObject.h"

CPrototype::CPrototype()
{
}

CPrototype::CPrototype(PROTOTYPE_SPEC&& tSpec)
    : m_tSpec(std::move(tSpec))
{
}

CPrototype::~CPrototype()
{
}

HRESULT CPrototype::Assemble(const ASSET_GUID& tGUID, PROTOTYPE_SPEC&& tSpec)
{
    m_tSpec = std::move(tSpec);

    /* 컴포넌트 비트 마스크 */
    m_componentMask = 0;

    for (uint32_t i = 0; i < SCAST(uint32_t, COMPONENT_MAX); ++i)
    {
        const auto& vSpecs = m_tSpec.tComponentBundle.components[i];
        if (vSpecs.empty())
            continue;

        /* 슬롯에 스펙이 1개라도 있으면 해당 타입 존재로 간주 */
        _bool hasValid = false;
        for (const auto& pSpec : vSpecs)
        {
            if (pSpec.get() != nullptr)
            {
                hasValid = true;
                break;
            }
        }

        if (hasValid)
            m_componentMask |= Component::To_Bit(INT_TO_COM(i));
    }

    m_tGUID = tGUID;
    m_bAssembled = true;
    return S_OK;
}

CGameObject* CPrototype::Clone(Layer::LAYER_ID iLayer, const string& strName, const INSTANCE_UUID& tUUID) const
{
    /* Create instance */
    CGameObject* pInstance = nullptr;
    if (!m_tSpec.isUI)
        pInstance = SYS_GAMEOBJECT.Create_GameObject(iLayer, strName, nullptr, tUUID);
    else
        pInstance = SYS_GAMEOBJECT.Create_GameObjectUI(iLayer, strName, nullptr, tUUID);

    SYS_GAMEOBJECT.Access_Data_Raw(pInstance->Get_Handle()).tProtoGUID = m_tGUID;

    CHECK_PROTO_CLONE_FAIL(FAILED(Apply_Spec_To_Instance(pInstance)), "CPrototype clone failed : can't apply spec to instance.");
    CHECK_PROTO_CLONE_FAIL(FAILED(Clone_Children(pInstance)), "CPrototype clone failed : child instantiation failed.");

    return pInstance;
}

HRESULT CPrototype::Apply_Spec_To_Instance(CGameObject* pInstance) const
{
    if (!pInstance)
        return E_FAIL;

    for (uint32_t i = 0; i < SCAST(uint32_t, COMPONENT_MAX); ++i)
    {
        const auto& vSpecs = m_tSpec.tComponentBundle.components[i];
        if (vSpecs.empty())
            continue;

        for (const auto& pSpec : vSpecs)
        {
            if (!pSpec.get())
                continue;

            SYS_COMPONENT.Create_Component_From_Spec(pInstance, pSpec.get());
        }
    }

    pInstance->Set_ComponentMask(m_componentMask);

    return S_OK;
}

HRESULT CPrototype::Clone_Children(CGameObject* pParent) const
{
    if (!pParent)
        return E_FAIL;

    /* 자식 스펙 기반으로 재귀적으로 인스턴스 생성 */
    const auto Spawn_From_Spec = [this](auto&& self, const PROTOTYPE_SPEC& tSpec, CGameObject* pAttachParent) -> CGameObject*
        {
            if (!pAttachParent)
                return nullptr;

            /* 부모 레이어를 그대로 상속 */
            const Layer::LAYER_ID iLayer = pAttachParent->Get_Layer();
            const string& strName = tSpec.strName;

            CGameObject* pChild = nullptr; 
            if (!tSpec.isUI)
                pChild = SYS_GAMEOBJECT.Create_GameObject(iLayer, strName, pAttachParent, INSTANCE_UUID{}); /* UUID 필요 없음 */
            else
                pChild = SYS_GAMEOBJECT.Create_GameObjectUI(iLayer, strName, pAttachParent, INSTANCE_UUID{});

            if (!pChild)
                return nullptr;

            /* NOTE : 자식도 같은 프로토타입에서 파생된 것으로 기록 */
            SYS_GAMEOBJECT.Access_Data_Raw(pChild->Get_Handle()).tProtoGUID = m_tGUID;

            /* 컴포넌트 생성 및 마스크 구성 */
            Component::COMPONENT_MASK childMask = 0;

            for (uint32_t i = 0; i < SCAST(uint32_t, COMPONENT_MAX); ++i)
            {
                const auto& vSpecs = tSpec.tComponentBundle.components[i];
                if (vSpecs.empty())
                    continue;

                _bool hasValid = false;

                for (const auto& pSpec : vSpecs)
                {
                    if (!pSpec.get())
                        continue;

                    hasValid = true;
                    SYS_COMPONENT.Create_Component_From_Spec(pChild, pSpec.get());
                }

                if (hasValid)
                    childMask |= Component::To_Bit(INT_TO_COM(i));
            }

            pChild->Set_ComponentMask(childMask);

            /* 손자들 재귀 생성 */ 
            for (const auto& childSpec : tSpec.vecChildren)
            {
                CGameObject* pGrandChild = self(self, childSpec, pChild);
                if (!pGrandChild)
                    return nullptr;
            }

            return pChild;
        };

    /* 루트의 직계 자식들 생성 */
    for (const auto& childSpec : m_tSpec.vecChildren)
    {
        CGameObject* pChild = Spawn_From_Spec(Spawn_From_Spec, childSpec, pParent);

        IF_NULL_RETURN_MSG_BREAK(pChild, E_FAIL, "Clone Children failed");
    }

    return S_OK;
}

std::unique_ptr<CPrototype> CPrototype::Create()
{
    return std::make_unique<CPrototype>();
}
