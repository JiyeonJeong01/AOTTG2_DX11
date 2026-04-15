#include "ThrownBlade.h"

void CThrownBlade::Awake(void* pCtx)
{
    
}

void CThrownBlade::Start(void* pCtx)
{
    m_pThrownBlade = GAME_INSTANCE.Find_GameObject(m_hObject);
    IF_NULL_RETURN_MSG_BREAK(m_pThrownBlade, , "m_upLeftBlade is nullptr");

    m_trBlade = m_pThrownBlade->Get_Component<CTransform>();

    CCollider cldr = m_pThrownBlade->Get_Component<CCollider>();
    cldr->OnTriggerEnter.Add_Listener(&CThrownBlade::On_TriggerEnter, this);
}

void CThrownBlade::Priority_Update(void* pCtx, _float fDT)
{

}

void CThrownBlade::Update(void* pCtx, _float fDT)
{

}

void CThrownBlade::Late_Update(void* pCtx, _float fDT)
{
    if (!m_bStarted)
        return;
    /* 회전 */
    _vector vBladeAxis = m_trBlade.Get_StateXM(STATE::RIGHT);
    m_trBlade.Rotate(vBladeAxis, m_fRotPerSec * fDT);

    /* 이동 */
    _vector vDir = XMLoadFloat3(&m_vDir);
    m_trBlade.Translate(vDir * m_fThrowSpeed * fDT, SPACE::WORLD);
}

void CThrownBlade::Start_Throw(_fvector vStartPoint, _fvector vDir)
{
    m_pThrownBlade->Set_Enable(true);

    XMStoreFloat3(&m_vStartPoint, vStartPoint);
    XMStoreFloat3(&m_vDir, vDir);

    m_trBlade.Set_Position(vStartPoint);

    _float3 vCamLook = GAME_INSTANCE.Cam_Look();
    m_trBlade.Look_At(XMLoadFloat3(&vCamLook));

    m_bStarted = true;
}

void CThrownBlade::On_TriggerEnter(const COLLISION_DESC& tDesc)
{
    CGameObject* pOther = GAME_INSTANCE.Find_GameObject(tDesc.hObject);
    if (!pOther)
        return;

    if (pOther->Has_Mask(O_PLAYER | O_SCOUT | O_EREN))
        return;

    if (!tDesc.pCounterCollider)
        return;

    if (!pOther->Has_Mask(O_HURTBOX) && tDesc.pCounterCollider->bTrigger)
        return;

    m_bStarted = false;

    m_pThrownBlade->Set_Enable(false);
}
