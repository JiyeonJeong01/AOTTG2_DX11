#pragma once
#include "Client_Define.h"
#include "Script.h"

NS_BEGIN(Client)

class CHitBox : public IScript
{
public :
    CHitBox();
    ~CHitBox();

    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

private :
    CGameObject*    m_goAttacker{};
    CGameObject*    m_goHitBox{};
    CCollider       m_cldrHit;

    HIT_INFO        m_tHitInfo{};

public :
    CGameObject*    Get_HitBoxObject() const;
    void            Set_Active(_bool bActive);
    _bool           Get_Active() const;

    void OnTriggerEnter(const COLLISION_DESC& tDesc);

private :
    SCRIPT_OBJECT_REF   m_refOwner;

SCRIPT_FIELDS_BEGIN(CHitBox)
    SCRIPT_FIELD_OBJECT_REF(m_refOwner)
    SCRIPT_FIELD_FLOAT(m_tHitInfo.fDamage)
SCRIPT_FIELDS_END(CHitBox)
};

NS_END;
