#pragma once
#include "Component_Processor_Impl.h"
#include "TestComponentA.h"

NS_BEGIN(Engine)

class ENGINE_DLL CTestComponentASystem final : public CComponent_Processor_Impl<CTestComponentA>
{
public:
	HRESULT Initialize() override;
	void Update(_float fDT) override;
	void LateUpdate(_float fDT) override;

    HRESULT Initialize_From_Spec(COMPONENT_HANDLE handle, const COMPONENT_SPEC_BASE* pSpec) override;

	/* 각 컴포넌트에 필요한 로직들 */
private :
	void Process_A(_float fDT);

public :
    static CTestComponentASystem* Create();
    void Free() override;
};

NS_END
