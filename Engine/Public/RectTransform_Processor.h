#pragma once
#include "Component_Processor_Impl.h"
#include "RectTransform.h"   // 너가 만든 Proxy/Data 헤더
NS_BEGIN(Engine)

class ENGINE_DLL CRectTransform_Processor final
    : public CComponent_Processor_Impl<CRectTransform, COMPONENT_TYPE::RECT_TRANSFORM>
{
    DEF_PROCESSOR_ID(PROCESSOR_ID::RECT_TRANSFORM)
public:
    HRESULT Initialize() override;
    void    Update(_float fDT) override;
    void    LateUpdate(_float fDT) override;

    HRESULT Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle, const COMPONENT_SPEC_BASE* pSpec) override;
    std::unique_ptr<COMPONENT_SPEC_BASE> Build_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent) override;

private:
    inline void Bake_World(RECTTRANSFORM_DATA* pData);

private :
    _float m_fWidth{}, m_fHeight{};

public:
    static std::unique_ptr<CRectTransform_Processor> Create();
};

NS_END
