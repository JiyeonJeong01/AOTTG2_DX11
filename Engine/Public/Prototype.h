#pragma once

#include "Spec_Struct.h"

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(Engine)

class ENGINE_DLL CPrototype final
{
public:
    CPrototype();
    explicit CPrototype(PROTOTYPE_SPEC&& tSpec);
    ~CPrototype();

public:
    const std::string& Get_Name() const noexcept
    {
        return m_tSpec.strName;
    }
    _bool Is_Assembled() const noexcept
    {
        return m_bAssembled;
    }
    _bool Has(COMPONENT_TYPE t) const noexcept
    {
        return (m_componentMask & Component::Component_Bit(t)) != 0;
    }

    HRESULT Assemble(PROTOTYPE_SPEC&& tSpec);
    CGameObject* Clone(Layer::LAYER_ID iLayer = Layer::DEFAULT_LAYER) const;

private:
    HRESULT Apply_Spec_To_Instance(CGameObject* pInstance) const;
    HRESULT Clone_Children(CGameObject* pParent) const;

private:
    PROTOTYPE_SPEC  m_tSpec{};
    _bool           m_bAssembled = false;
    Component::COMPONENT_MASK  m_componentMask = 0;

public :
    static std::unique_ptr<CPrototype> Create();
};

#define CHECK_PROTO_CLONE_FAIL(condition, message)      \
    if (condition) {                                    \
        _DEBUG_ERROR_BREAK(message);                    \
        SYS_GAMEOBJECT.Destroy_Object(pInstance);       \
        return nullptr;                                 \
    }

NS_END
