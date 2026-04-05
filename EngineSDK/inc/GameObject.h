#pragma once

#include "GameObject_System.h"

NS_BEGIN(Engine)

class CGameObject_System;

class ENGINE_DLL CGameObject : public LABEL
{
    friend class CGameObject_System;
public:
    CGameObject(uint32_t iPoolIndex);
    ~CGameObject();

public:
    void Render();

protected:
    OBJECT_HANDLE       m_hSelf{};
    uint32_t            m_iMask{};

public:
    /* Components */
    template <typename TProxy>
    TProxy Add_Component();

    template <typename TProxy>
    TProxy Get_Component();

    template <typename TProxy>
    std::vector<TProxy> Get_Components();

    template <typename TScript>
    TScript* Get_Script();

    template <typename TScript>
    TScript* Get_Script_InChildren();

    template <typename TScript>
    std::vector<TScript*> Get_AllScripts_InChildren();

    void                Remove_Components(COMPONENT_TYPE eComType);
    void                Remove_All_Components();

    /* Hierarchy */
    CGameObject*                Get_Parent();
    HRESULT                     Set_Parent(CGameObject* pNewParent);
    HRESULT                     Add_Child(CGameObject* pChild);
    HRESULT                     Remove_Child(CGameObject* pChild);

    std::vector<CGameObject*>   Get_Children() const;

    OBJECT_HANDLE               Get_Handle() const;
    _bool                       Is_Valid() const;

    /* etc */
    void                        Set_Enable(_bool bActive);
    _bool                       Get_Enabled() const;

    template <typename T>
    void                        Add_Mask(T eMask);
    template <typename T>
    void                        Remove_Mask(T eMask);
    template <typename T>
    bool                        Has_Mask(T eMask) const;
    template <typename T>
    void                        Set_Mask(T eMask);
    uint32_t                    Get_Mask() const;

    void                        Set_ComponentMask(Component::COMPONENT_MASK mask);
    Component::COMPONENT_MASK   Get_ComponentMask() const;

    Layer::LAYER_ID             Get_Layer() const;

    const INSTANCE_UUID&        Get_UUID() const;
    const ASSET_GUID&           Get_ProtoGUID() const;
    void                        Set_ProtoGUID(const ASSET_GUID& tGUID);

private:
    void Add_Child_Inner(OBJECT_HANDLE hChild);
    void Remove_Child_Inner(OBJECT_HANDLE hChild);

public:
    CGameObject* Clone();
};

NS_END

#include "GameObject.inl"
