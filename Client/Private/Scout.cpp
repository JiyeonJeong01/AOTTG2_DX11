#include "Scout.h"

NS_BEGIN(Client)

void CScout::Awake(void* pCtx)
{
    CGameObject* pOwner = GAME_INSTANCE.Find_GameObject(m_hObject);

    m_trOwner = pOwner->Get_Component<CTransform>();
    IF_TRUE_RETURN_MSG_BREAK(!m_trOwner.Is_Valid(), , "m_trOwner is invalid");

    m_cldrOwner = pOwner->Get_Component<CCollider>();
    IF_TRUE_RETURN_MSG_BREAK(!m_cldrOwner.Is_Valid(), , "m_cldrOwner is invalid");

    m_animOwner = pOwner->Get_Component<CAnimator>();
    IF_TRUE_RETURN_MSG_BREAK(!m_animOwner.Is_Valid(), , "m_animOwner is invalid");

    m_mrOwner = pOwner->Get_Component<CMeshRenderer>();
    IF_TRUE_RETURN_MSG_BREAK(!m_mrOwner.Is_Valid(), , "m_mrOwner is invalid");
}

void CScout::Start(void* pCtx)
{
}

void CScout::Priority_Update(void* pCtx, _float fDT)
{
}

void CScout::Update(void* pCtx, _float fDT)
{
}

void CScout::Late_Update(void* pCtx, _float fDT)
{
}

void CScout::Move(_fvector vDir, _float fDT)
{

}

NS_END;
