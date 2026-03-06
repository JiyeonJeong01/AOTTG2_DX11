#pragma once
#include "Component_Processor.h"
#include "Component_Pool.h"

#include "Collider.h"


NS_BEGIN(Engine)
class CTransform_Processor;

class CPhysics_Processor :  public CComponent_Processor
{
    DEF_PROCESSOR_ID(PROCESSOR_ID::PHYSICS)
public:
    HRESULT Initialize() override;
    void    LateUpdate(_float fDT) override;
    void    Fixed_Update(_float fDT);
    void    Render();

    COMPONENT_HANDLE    Create_Component_Data(COMPONENT_TYPE eComType, OBJECT_HANDLE hObject) override;
    void                Remove_Component(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent) override;
    HRESULT             Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent, const COMPONENT_SPEC_BASE* pSpec) override;
    std::unique_ptr<COMPONENT_SPEC_BASE> Build_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent) override;
    void                Set_Enable(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent, _bool bEnable) override;
    void*               Get_DataPtr(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent) noexcept override;

    template<typename TProxy>
    TProxy Get_Proxy(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent)
    {
        if constexpr (std::is_same_v<TProxy, CCollider>) {
            return m_ColliderPool.Get_Proxy(hComponent);
        }

        IF_TRUE_RETURN_MSG_BREAK(true, TProxy{}, "Invalid Proxy Type for this Pool");
    }

private :
    template <typename TProxy>
    COMPONENT_HANDLE Create_Component_Data_Inner(CComponent_Pool<TProxy>& pool, OBJECT_HANDLE hObject)
    {
        COMPONENT_HANDLE hComponent = pool.Allocate();
        auto pData = pool.Get_Data_By_Handle(hComponent);
        pData->hObject = hObject;

        Initialize_Component_Data(TProxy::ComponentType, hComponent);

        return hComponent;
    }

    template <typename TProxy>
    void Remove_Component_Inner(CComponent_Pool<TProxy>& pool, COMPONENT_HANDLE hComponent)
    {
        pool.Deallocate(hComponent);
    }

    template <typename TProxy>
    void Set_Enable_Inner(CComponent_Pool<TProxy>& pool, COMPONENT_HANDLE hComponent, _bool bEnable)
    {
        pool.Get_Data_By_Handle(hComponent)->bEnable = bEnable;
    }

private :
    HRESULT Initialize_From_Spec_Collider(COMPONENT_HANDLE h, const COMPONENT_SPEC_BASE* spec);
    HRESULT Initialize_Component_Data(COMPONENT_TYPE eComType, COMPONENT_HANDLE h);

public :

private:
    CTransform_Processor* m_pTransformProcessor{};
    CComponent_Pool<CCollider>  m_ColliderPool;

    std::unique_ptr<class CCollision_Detector>      m_upCollision_Detector{};
    std::unique_ptr<class CCollider_Proxy_Builder>   m_upCollider_Builder{};

    std::vector<COLLIDER_PROXY_DATA> m_AllColliders{}; /* this tick */

public :
    static std::unique_ptr<CPhysics_Processor> Create();
};

NS_END
