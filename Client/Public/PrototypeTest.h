#pragma once
#include "Client_Define.h"
#include "Script.h"


namespace Engine
{
    class CGameObject;
}

NS_BEGIN(Client)
    class CPrototypeTest : public IScript
{

public:
    _int                m_iSpeed = 0;
    SCRIPT_OBJECT_REF   m_rObject{};

public:
    SCRIPT_FIELDS_BEGIN(CPrototypeTest)
        SCRIPT_FIELD_INT(m_iSpeed)
        SCRIPT_FIELD_OBJECT_REF(m_rObject)
    SCRIPT_FIELDS_END(CPrototypeTest)

public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

private :
    CGameObject* m_pTarget = nullptr;

};

NS_END;
