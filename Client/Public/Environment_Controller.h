#pragma once
#include "Client_Define.h"
#include "Script.h"

NS_BEGIN(Client)

class CEnvironment_Controller : public IScript
{
public:
    CEnvironment_Controller();
    ~CEnvironment_Controller();

    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

private :
    CMeshRenderer               m_mrWater;
    uint32_t                    m_hPerObjBlockWater = 0;
    PER_OBJECT_PARAM_BLOCK*     m_pBlockWater = nullptr;

    _float                      m_fDT = 0.f;

public:
    SCRIPT_OBJECT_REF   m_refWater{};

    SCRIPT_FIELDS_BEGIN(CEnvironment_Controller)
        SCRIPT_FIELD_OBJECT_REF(m_refWater)
    SCRIPT_FIELDS_END(CEnvironment_Controller)

};

NS_END;
