#pragma once
#include "Client_Define.h"
#include "Script.h"

NS_BEGIN(Client)

class CGroundChecker : public IScript
{
    


private :
    SCRIPT_OBJECT_REF   m_refOwner{};

public:
    SCRIPT_FIELDS_BEGIN(CGroundChecker)
        SCRIPT_FIELD_OBJECT_REF(m_refOwner)
    SCRIPT_FIELDS_END(CGroundChecker)

private :
    CGameObject*        m_pOwner{};
    CGameObject*        m_pChecker{};
    CTransform          m_trOwner;
    CTransform          m_trChecker;

public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;
};

NS_END;
