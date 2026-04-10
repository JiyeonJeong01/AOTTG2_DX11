#include "HitBox.h"
#include "HurtBox.h"

NS_BEGIN(Client)

CHitBox::CHitBox()
{
}

CHitBox::~CHitBox()
{
}

void CHitBox::Awake(void* pCtx)
{
    m_goAttacker = GAME_INSTANCE.Find_GameObject(m_refOwner.hObject);
    IF_NULL_RETURN_MSG_BREAK(m_goAttacker, , "m_goOwner is nullptr");

    m_goHitBox = GAME_INSTANCE.Find_GameObject(m_hObject);
    IF_NULL_RETURN_MSG_BREAK(m_goHitBox, , "pObj is nullptr");

    m_cldrHit = m_goHitBox->Get_Component<CCollider>();
    IF_TRUE_RETURN_MSG_BREAK(!m_cldrHit.Is_Valid(), , "m_cldrHit is invalidd");

    m_cldrHit->OnTriggerEnter.Add_Listener(&CHitBox::OnTriggerEnter, this);

    m_tHitInfo.goAttacker = m_goAttacker;

    m_trHitBox = m_goHitBox->Get_Component<CTransform>();
}

void CHitBox::Start(void* pCtx)
{
}

void CHitBox::Priority_Update(void* pCtx, _float fDT)
{
}

void CHitBox::Update(void* pCtx, _float fDT)
{
}

void CHitBox::Late_Update(void* pCtx, _float fDT)
{
}

CGameObject* CHitBox::Get_HitBoxObject() const
{
    return m_goHitBox;
}

void CHitBox::Set_Active(_bool bActive)
{
    m_cldrHit->bEnable = bActive;

    if (bActive)
        LOG_INFO("hitbox on");
    else
        LOG_INFO("hitbox off");
}

_bool CHitBox::Get_Active() const
{
    return m_cldrHit->bEnable;
}

void CHitBox::Set_Position(_fvector vPos)
{
    m_trHitBox.Set_Position(vPos);
}

void CHitBox::Set_TargetMask(_int iMask)
{
    m_iTargetMask |= iMask;
}

void CHitBox::Set_DiscardMask(_int iMask)
{
    m_iDiscardtMask |= iMask;
}

void CHitBox::OnTriggerEnter(const COLLISION_DESC& tDesc)
{
    if (!Get_Active())
        return;

    CGameObject* pOther = GAME_INSTANCE.Find_GameObject(tDesc.hObject);
    if (!pOther)
        return;

    if (pOther->Has_Mask(m_iDiscardtMask))
        return;

    if (!pOther->Has_Mask(O_HURTBOX) && !pOther->Has_Mask(m_iTargetMask))
        return;

    CHurtBox* pHurtBox = pOther->Get_Script<CHurtBox>();
    if (!pHurtBox)
        return;

    if (pOther == m_tHitInfo.goAttacker)
        return;

    HIT_INFO tInfo = m_tHitInfo;
    tInfo.vHitPoint = tDesc.vPoint;

    pHurtBox->Try_ApplyHit(tInfo);
    m_OnSuccessHit.Invoke(pOther);
}

NS_END;
