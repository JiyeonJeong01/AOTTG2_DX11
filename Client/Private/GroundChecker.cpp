#include "GroundChecker.h"

NS_BEGIN(Client)

void CGroundChecker::Awake(void* pCtx)
{
    /* 변수 값 할당 */
    m_pOwner = GAME_INSTANCE.Find_GameObject(m_refOwner.hObject);
    if (!m_pOwner)
        m_pOwner = GAME_INSTANCE.Find_GameObject(m_hObject);

    m_trOwner = m_pOwner->Get_Component<CTransform>();
    IF_TRUE_RETURN_MSG_BREAK(m_trOwner.Is_Valid() == false, , "transform is invalid");
}

void CGroundChecker::Start(void* pCtx)
{
}

void CGroundChecker::Priority_Update(void* pCtx, _float fDT)
{

}

void CGroundChecker::Update(void* pCtx, _float fDT)
{
}

void CGroundChecker::Late_Update(void* pCtx, _float fDT)
{
    /* transform 동기화 */
    m_trChecker->vPosition = m_trOwner->vPosition;
    m_trChecker->vScale = m_trOwner->vScale;
    m_trChecker->vRotationQuat = m_trOwner->vRotationQuat;
}

NS_END;
