#include "Prototype.h"
#include "Engine_Log.h"
#include "GameObject_System.h"
#include "Component_System.h"
#include "GameObject.h"


HRESULT CPrototype::Assemble(PROTOTYPE_SPEC && tSpec)
{
    m_tSpec = std::move(tSpec);

    /* Update component bitmask to mark the presence of this type */
    m_componentMask = 0;
    for (auto* pSpec : m_tSpec.tComponentBundle.components)
    {
        const COMPONENT_TYPE eComType = pSpec->Get_Type();
        if (eComType == COMPONENT_TYPE::END)
            continue;

        const uint32_t iIdx = SCAST(uint32_t, eComType);
        if (iIdx < SCAST(uint32_t, COMPONENT_MAX))
            m_componentMask |= Component::Component_Bit(eComType);
    }

    m_bAssembled = true;
    return S_OK;
}

CGameObject* CPrototype::Clone() const
{
    /* Create instance */
    CGameObject* pInstance = SYS_GAMEOBJECT.Create_Object(m_tSpec.layer, m_tSpec.strName);

    CHECK_PROTO_CLONE_FAIL(!pInstance, "CPrototype clone failed : instance is nullptr.");
    CHECK_PROTO_CLONE_FAIL(!pInstance->IsValid(), "CPrototype clone failed : instance is not valid.");
    CHECK_PROTO_CLONE_FAIL(FAILED(Apply_Spec_To_Instance(pInstance)), "CPrototype clone failed : can't apply spec to instance.");
    CHECK_PROTO_CLONE_FAIL(FAILED(Clone_Children(pInstance)), "CPrototype clone failed : child instantiation failed.");

    return pInstance;
}

HRESULT CPrototype::Apply_Spec_To_Instance(CGameObject* pInstance) const
{
    if (!pInstance)
        return E_FAIL;

    for (auto* pSpec : m_tSpec.tComponentBundle.components)
    {
        if (!pSpec)
            return E_FAIL;

        SYS_COMPONENT.Create_From_Spec(pInstance, pSpec);
    }

    pInstance->Set_ComponentMask(m_componentMask);

    return S_OK;
}

HRESULT CPrototype::Clone_Children(CGameObject* pParent) const
{
    //if (!pParent)
    //    return E_FAIL;

    //for (const auto& childSpec : m_tSpec.children)
    //{
    //    CPrototype childProto(childSpec);

    //    PROTOTYPE_SPEC tmp = childSpec;
    //    childProto.Assemble(std::move(tmp));

    //    CGameObject* pChild = nullptr;
    //    if (FAILED(childProto.Clone(&pChild)) || !pChild)
    //    {
    //        _DEBUG_ERROR_BREAK("Clone Children failed");
    //        return E_FAIL;
    //    }

    //    pChild->Set_Parent(pParent);
    //}

    return S_OK;
}

CPrototype* CPrototype::Create()
{
    return new CPrototype;
}
