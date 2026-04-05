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
    m_OnHit.Invoke(tHitBox);
}

NS_END;
