#pragma once
#include <Transform.h>
#include "SpringJoint.h"
#include "Client_Define.h"
#include "Script.h"


namespace Engine
{
    class CGameObject;
}

NS_BEGIN(Client)
    class CHello : public IScript
{
public :
    _int                m_iSpeed = 0;
    _float3             m_vDir = { 0.f, 0.f, 0.f };
    SCRIPT_OBJECT_REF   m_rObject{};

public :
    SCRIPT_FIELDS_BEGIN(CHello)
        SCRIPT_FIELD_INT(m_iSpeed)
        SCRIPT_FIELD_FLOAT3(m_vDir)
        SCRIPT_FIELD_OBJECT_REF(m_rObject)
    SCRIPT_FIELDS_END(CHello)


public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

private :
    CTransform      m_Transform;
    CSpringJoint    m_SpringJoint;
    CGameObject* m_pTarget = nullptr;

private :
    void Move(_float fDT);
};

NS_END;
