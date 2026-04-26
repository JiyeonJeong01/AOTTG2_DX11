#include "Titan_Scriptable_Object.h"
#include "GameInstance.h"

NS_BEGIN(Client)

void CTitan_Scriptable_Object::Awake(void* pCtx)
{
    UNREFERENCED_PARAMETER(pCtx);

    Sync_To_SO();
}

void CTitan_Scriptable_Object::Start(void* pCtx)
{
    UNREFERENCED_PARAMETER(pCtx);

    Sync_To_SO();
}

void CTitan_Scriptable_Object::Priority_Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(pCtx);
    UNREFERENCED_PARAMETER(fDT);
}

void CTitan_Scriptable_Object::Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(pCtx);
    UNREFERENCED_PARAMETER(fDT);
}

void CTitan_Scriptable_Object::Late_Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(pCtx);
    UNREFERENCED_PARAMETER(fDT);
}

const TITAN_SCRIPTABLE_OBJECT& CTitan_Scriptable_Object::Get_Data() const
{
    return m_tSO;
}

CGameObject* CTitan_Scriptable_Object::Get_TitanObject() const
{
    if (!m_refTitan.Is_Valid())
        return nullptr;

    return GAME_INSTANCE.Find_GameObject(m_refTitan.hObject);
}

void CTitan_Scriptable_Object::Sync_To_SO()
{
    m_tSO.fCurSpeed = m_fCurSpeed;
    m_tSO.fMaxSpeed = m_fMaxSpeed;

    m_tSO.goEren = GAME_INSTANCE.Find_GameObject(m_refEren.hObject);

    memset(m_tSO.szMoveAnim, 0, sizeof(m_tSO.szMoveAnim));
    memset(m_tSO.szIdleAnim, 0, sizeof(m_tSO.szIdleAnim));

    strncpy_s(m_tSO.szMoveAnim, m_szMoveAnim, _TRUNCATE);
    strncpy_s(m_tSO.szIdleAnim, m_szIdleAnim, _TRUNCATE);

    m_tSO.fMaxIdleTime = m_fMaxIdleTime;
    m_tSO.fMaxMoveTime = m_fMaxMoveTime;

    m_tSO.iHitEffect = m_iHitEffect;
    m_tSO.iHurtSound = m_iHurtSound;
}

NS_END
