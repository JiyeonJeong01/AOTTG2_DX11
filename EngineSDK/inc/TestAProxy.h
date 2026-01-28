#pragma once
#include "Component.h"

NS_BEGIN(Engine)

struct TEST_DATA_A {
    _float3 vData[4];
    _float  fAcc;
};

class ENGINE_DLL CTestAProxy final : public CComponent
{
public:
    CTestAProxy() : CComponent(COMPONENT_TYPE::TEST_A) {}
    CTestAProxy(TEST_DATA_A* pRawData) : CComponent(COMPONENT_TYPE::TEST_A), m_pData(pRawData) {}
    virtual ~CTestAProxy() override = default;

public:
    void Set_Data_Pointer(TEST_DATA_A* pData, COMPONENT_HANDLE hHandle)
	{
        m_pData = pData;
        m_hMyHandle = hHandle;
    }

    void Add_Acc(float fValue) { if (m_pData) m_pData->fAcc += fValue; }

private:
    TEST_DATA_A* m_pData = nullptr;    
    COMPONENT_HANDLE  m_hMyHandle;     
};

NS_END