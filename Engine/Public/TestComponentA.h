#pragma once
#include "CComponent_Proxy_Base.h"

NS_BEGIN(Engine)

struct TEST_DATA_A {
    _float3 vData[4];
    _float  fAcc;
};

class ENGINE_DLL CTestComponentA final : public CComponent_Proxy_Base<TEST_DATA_A, CTestComponentA>
{
public:
    CTestComponentA() : CComponent_Proxy_Base() { m_eComType = COMPONENT_TYPE::TEST_A;  }
    CTestComponentA(TEST_DATA_A* pData, COMPONENT_HANDLE handle) : CComponent_Proxy_Base(pData, handle) { m_eComType = COMPONENT_TYPE::TEST_A;  }
    ~CTestComponentA() override = default;

    void Add_Acc(float fValue) { if (m_pData) m_pData->fAcc += fValue; }
};

NS_END
