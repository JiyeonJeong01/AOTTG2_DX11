#include "ResupplyStation.h"

NS_BEGIN(Client)

CResupplyStation::CResupplyStation()
{
}

CResupplyStation::~CResupplyStation()
{
}

void CResupplyStation::Awake(void* pCtx)
{
    UNREFERENCED_PARAMETER(pCtx);

    m_goOwner = GAME_INSTANCE.Find_GameObject(m_hObject);
    IF_NULL_RETURN_MSG_BREAK(m_goOwner, , "m_goOwner is nullptr");

    m_trOwner = m_goOwner->Get_Component<CTransform>();
    IF_TRUE_RETURN_MSG_BREAK(!m_trOwner.Is_Valid(), , "m_trOwner is invalid");

    m_goTriggerObject = GAME_INSTANCE.Find_GameObject(m_refTriggerObject.hObject);
    IF_NULL_RETURN_MSG_BREAK(m_goTriggerObject, , "m_goTriggerObject is nullptr");

    m_trTrigger = m_goTriggerObject->Get_Component<CTransform>();
    IF_TRUE_RETURN_MSG_BREAK(!m_trTrigger.Is_Valid(), , "m_trTrigger is invalid");

    m_clTrigger = m_goTriggerObject->Get_Component<CCollider>();
    IF_TRUE_RETURN_MSG_BREAK(!m_clTrigger.Is_Valid(), , "m_clTrigger is invalid");

    m_goUIPivotObject = GAME_INSTANCE.Find_GameObject(m_refUIPivotObject.hObject);
    if (m_goUIPivotObject != nullptr)
    {
        m_trUIPivot = m_goUIPivotObject->Get_Component<CTransform>();
    }

    m_clTrigger->OnTriggerEnter.Add_Listener(&CResupplyStation::OnTriggerEnter, this);
    m_clTrigger->OnTriggerExit.Add_Listener(&CResupplyStation::OnTriggerExit, this);
}

void CResupplyStation::Start(void* pCtx)
{
    UNREFERENCED_PARAMETER(pCtx);
}

void CResupplyStation::Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(pCtx);
    UNREFERENCED_PARAMETER(fDT);
}

void CResupplyStation::Late_Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(pCtx);
    UNREFERENCED_PARAMETER(fDT);

    if (!m_bPlayerDetected)
        return;

    if (m_goDetectedPlayer == nullptr)
        return;

    /* F 상호작용 붙일 자리  */
    if (SYS_INPUT.Get_Key('F'))
    {
    }

}

_float3 CResupplyStation::Get_UIWorldPosition() const
{
    if (m_trUIPivot.Is_Valid())
        return m_trUIPivot->vPosition;

    if (m_trTrigger.Is_Valid())
        return m_trTrigger->vPosition;

    if (m_trOwner.Is_Valid())
        return m_trOwner->vPosition;

    return _float3(0.f, 0.f, 0.f);
}

void CResupplyStation::OnTriggerEnter(const COLLISION_DESC& tCollisionDesc)
{
    CGameObject* pOther = GAME_INSTANCE.Find_GameObject(tCollisionDesc.hObject);
    if (!Is_Player(pOther))
        return;

    m_goDetectedPlayer = pOther;
    m_bPlayerDetected = true;

    On_DetectedPlayer(true, pOther);
}

void CResupplyStation::OnTriggerExit(const COLLISION_DESC& tCollisionDesc)
{
    CGameObject* pOther = GAME_INSTANCE.Find_GameObject(tCollisionDesc.hObject);
    if (!Is_Player(pOther))
        return;

    if (m_goDetectedPlayer != pOther)
        return;

    m_goDetectedPlayer = nullptr;
    m_bPlayerDetected = false;

    On_DetectedPlayer(false, pOther);
}

_bool CResupplyStation::Is_Player(const CGameObject* pOther) const
{
    if (pOther == nullptr)
        return false;

    return pOther->Has_Mask(O_PLAYER);
}

void CResupplyStation::On_DetectedPlayer(_bool bDetected, CGameObject* pPlayer)
{
    UNREFERENCED_PARAMETER(pPlayer);

    if (bDetected)
    {
        /* UI Show 요청 지점 */
    }
    else
    {
        /* UI Hide 요청 지점 */
    }
}

NS_END
