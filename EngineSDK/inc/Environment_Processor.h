// Environment_System.h
#pragma once
#include "Component_Pool.h"
#include "Component_Processor.h"
#include "Camera.h"
#include "Light.h"

NS_BEGIN(Engine)

class ENGINE_DLL CEnvironment_Processor final : public CComponent_Processor
{
public:
    CEnvironment_Processor();
    ~CEnvironment_Processor() override;

public:
    HRESULT Initialize() override;
    void    Update(_float fDT) override;
    void    LateUpdate(_float fDT) override;

    COMPONENT_HANDLE    Create_Component_Data(COMPONENT_TYPE eComType, OBJECT_HANDLE hObject) override;
    void                Remove_Component(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent) override;
    HRESULT             Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent, const COMPONENT_SPEC_BASE* pSpec) override;
    std::unique_ptr<COMPONENT_SPEC_BASE> Build_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent) override;

public:
    template<typename TProxy>
    TProxy Get_Proxy(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent)
    {
        if constexpr (std::is_same_v<TProxy, CCamera>) {
            return m_CameraPool.Get_Proxy(hComponent);
        }
        else if constexpr (std::is_same_v<TProxy, CLight>) {
            return m_LightPool.Get_Proxy(hComponent);
        }

        IF_TRUE_RETURN_MSG_BREAK(true, TProxy{}, "Invalid Proxy Type for this Pool");
    }

private:
    template <typename TProxy>
    COMPONENT_HANDLE Create_Component_Data_Inner(CComponent_Pool<TProxy>& pool, OBJECT_HANDLE hObject)
    {
        COMPONENT_HANDLE hComponent = pool.Allocate();
        auto pData = pool.Get_Data_By_Handle(hComponent);
        pData->hObject = hObject;
        return hComponent;
    }

    template <typename TProxy>
    void Remove_Component_Inner(CComponent_Pool<TProxy>& pool, COMPONENT_HANDLE hComponent)
    {
        pool.Deallocate(hComponent);
    }

private:
    HRESULT Initialize_From_Spec_Camera(COMPONENT_HANDLE h, const COMPONENT_SPEC_BASE* spec);
    HRESULT Initialize_From_Spec_Light(COMPONENT_HANDLE h, const COMPONENT_SPEC_BASE* spec);

    std::unique_ptr<COMPONENT_SPEC_BASE> Build_Spec_Camera(COMPONENT_HANDLE h);
    std::unique_ptr<COMPONENT_SPEC_BASE> Build_Spec_Light(COMPONENT_HANDLE h);

private:
    CComponent_Pool<CCamera> m_CameraPool;
    CComponent_Pool<CLight> m_LightPool;

public:
    static std::unique_ptr<CEnvironment_Processor> Create();
};

NS_END
