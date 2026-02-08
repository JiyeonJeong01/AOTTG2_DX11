#pragma once
#include "Component_Processor_Impl.h"
#include "TestComponentB.h"

NS_BEGIN(Engine)

class ENGINE_DLL CTestComponentBSystem final : public CComponent_Processor_Impl<CTestComponentB>
{
public:
	HRESULT Initialize() override;
	void Update(_float fDT) override;
	void LateUpdate(_float fDT) override;

	/* 각 컴포넌트에 필요한 로직들 */
private:
	void Process_B(_float fDT);

public:
    static CTestComponentBSystem* Create();
    void Free() override;

};

NS_END
