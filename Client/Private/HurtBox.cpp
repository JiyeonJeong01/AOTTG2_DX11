#include "HurtBox.h"

NS_BEGIN(Client)

CHurtBox::CHurtBox()
{
}

CHurtBox::~CHurtBox()
{
}

void CHurtBox::Awake(void* pCtx)
{
    m_pOwner = GAME_INSTANCE.Find_GameObject(m_refOwner.hObject);
    IF_NULL_RETURN_MSG_BREAK(m_pOwner, , "m_pOwner is nullptr");

    m_goHurtBox = GAME_INSTANCE.Find_GameObject(m_hObject);
    IF_NULL_RETURN_MSG_BREAK(m_goHurtBox, , "m_goHurtBox is nullptr");
}

void CHurtBox::Start(void* pCtx)
{
}

void CHurtBox::Priority_Update(void* pCtx, _float fDT)
{
}

void CHurtBox::Update(void* pCtx, _float fDT)
{
}

void CHurtBox::Late_Update(void* pCtx, _float fDT)
{
}

void CHurtBox::Try_ApplyHit(const HIT_INFO& tHitBox)
{
    CGameObject* pHurtBox = GAME_INSTANCE.Find_GameObject(m_hObject);
    IF_NULL_RETURN_MSG_BREAK(pHurtBox, , "pHurtBox is nullptr");

    m_OnHurt.Invoke(tHitBox, string(pHurtBox->Get_Label()));
}

CGameObject* CHurtBox::Get_Owner() const
{
    return m_pOwner;
}

CGameObject* CHurtBox::Get_HurtBoxObject() const
{
    return m_goHurtBox;
}

NS_END;
