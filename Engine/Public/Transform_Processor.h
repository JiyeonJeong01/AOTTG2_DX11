#pragma once
#include "Component_Processor_Impl.h"
#include "Transform.h"

NS_BEGIN(Engine)

class ENGINE_DLL CTransform_Processor final : public CComponent_Processor_Impl<CTransform>
{
public:
    HRESULT Initialize() override;
    void Update(_float fDT) override;
    void LateUpdate(_float fDT) override;

    HRESULT Initialize_From_Spec(COMPONENT_HANDLE handle, const COMPONENT_SPEC_BASE* pSpec) override;

    /* 각 컴포넌트에 필요한 로직들 */
private:
    inline void Bake_World(TRANSFORM_DATA* pData);

public:
    static std::unique_ptr<CTransform_Processor> Create();
};

NS_END
