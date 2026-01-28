#pragma once
#include "CComponent_Proxy_Base.h"

NS_BEGIN(Engine)

struct TEST_DATA_B {
    _float3 vData[4];
    _float  fAcc;
};

class ENGINE_DLL CTestComponentB final : public CComponent_Proxy_Base<TEST_DATA_B, CTestComponentB>
{
public:
    CTestComponentB() : CComponent_Proxy_Base(COMPONENT_TYPE::TEST_B) {}
    CTestComponentB(TEST_DATA_B* pData, COMPONENT_HANDLE handle) : CComponent_Proxy_Base(pData, handle) {};
    ~CTestComponentB() override = default;

public:
    void Add_Acc(float fValue) { if (m_pData) m_pData->fAcc += fValue; }
};

NS_END