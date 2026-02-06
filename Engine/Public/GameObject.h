#pragma once
#include "Base.h"
#include "GameObject_System.h"

NS_BEGIN(Engine)

class CGameObject_System;

class ENGINE_DLL CGameObject : public CBase, public LABEL
{
    friend class CGameObject_System;

protected:
    CGameObject(uint32_t iPoolIndex);
    ~CGameObject() override = default;

public:
    void Render();

protected:
    GAMEOBJECT_HANDLE     m_hSelf{};

public:
    /* Components */
    template <typename TProxy>
    TProxy Add_Component(COMPONENT_TYPE eComType);

    template <typename TProxy>
    TProxy Get_Component(COMPONENT_TYPE eComType);

    template <typename TProxy>
    std::vector<TProxy> Get_Components(COMPONENT_TYPE eComType);

    /* Hierarchy */
    CGameObject*                Get_Parent();
    HRESULT                     Set_Parent(CGameObject* pNewParent);
    HRESULT                     Add_Child(CGameObject* pChild);
    HRESULT                     Remove_Child(CGameObject* pChild);

    std::vector<CGameObject*>   Get_Children() const;

    GAMEOBJECT_HANDLE           Get_Handle() const;
    _bool                       IsValid() const;

    /* etc */
    void                        Set_Active(_bool bActive);
    _bool                       Get_Active() const;

    void                        Set_ComponentMask(Component::COMPONENT_MASK mask);
    Component::COMPONENT_MASK   Get_ComponentMask() const;

private:
    void Add_Child_Inner(GAMEOBJECT_HANDLE hChild);
    void Remove_Child_Inner(GAMEOBJECT_HANDLE hChild);

public:
    static CGameObject* Create(uint32_t iLayer = Layer::DEFAULT_LAYER, std::string strName = "GameObject", CGameObject* pParent = nullptr);
    CGameObject* Clone();

private:
    void Free() override;
};

NS_END

#include "GameObject.inl"
