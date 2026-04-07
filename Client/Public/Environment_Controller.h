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
    CMeshRenderer   m_mrRainy;


public:
    SCRIPT_OBJECT_REF   m_refRainy{};

    SCRIPT_FIELDS_BEGIN(CEnvironment_Controller)
        SCRIPT_FIELD_OBJECT_REF(m_refRainy)
    SCRIPT_FIELDS_END(CEnvironment_Controller)

};

NS_END;
