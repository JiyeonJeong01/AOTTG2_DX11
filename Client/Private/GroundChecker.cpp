#include "GroundChecker.h"

NS_BEGIN(Client)

_bool CGroundChecker::Get_OnWalkable() const
{
    return m_iGroundContactCount > 0;
}

void CGroundChecker::Awake(void* pCtx)
{
    /* 변수 값 할당 */
    /* 목표 오브젝트 */
    m_pOwner = GAME_INSTANCE.Find_GameObject(m_refOwner.hObject);
    if (!m_pOwner)
        m_pOwner = GAME_INSTANCE.Find_GameObject(m_hObject);

    m_trOwner = m_pOwner->Get_Component<CTransform>();
    IF_TRUE_RETURN_MSG_BREAK(m_trOwner.Is_Valid() == false, , "transform is invalid");

    /* 체커 오브젝트 */

    m_pChecker = GAME_INSTANCE.Find_GameObject(m_hObject);
    IF_NULL_RETURN_MSG_BREAK(m_pChecker, , "pChecker is nullptr");

    m_trChecker = m_pChecker->Get_Component<CTransform>();
    IF_TRUE_RETURN_MSG_BREAK(m_trChecker.Is_Valid() == false, , "transform is invalid");

    m_colChecker = m_pChecker->Get_Component<CCollider>();
    IF_TRUE_RETURN_MSG_BREAK(m_colChecker.Is_Valid() == false, , "collider is invalid");

    /* 충돌 이벤트 등록 */
    m_colChecker->OnTriggerEnter.Add_Listener(&CGroundChecker::OnTriggerEnter, this);
    m_colChecker->OnTriggerExit.Add_Listener(&CGroundChecker::OnTriggerExit, this);
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

void CGroundChecker::OnTriggerEnter(const COLLISION_DESC& tDesc)
{
    CGameObject* pCounter = GAME_INSTANCE.Find_GameObject(tDesc.hObject);
    if (pCounter && pCounter->Has_Mask(O_WALKABLE))
        m_iGroundContactCount++;
}

void CGroundChecker::OnTriggerExit(const COLLISION_DESC& tDesc)
{
    CGameObject* pCounter = GAME_INSTANCE.Find_GameObject(tDesc.hObject);
    if (pCounter && pCounter->Has_Mask(O_WALKABLE))
    {
        m_iGroundContactCount--;
        if (m_iGroundContactCount < 0) m_iGroundContactCount = 0;
    }
}

NS_END;
